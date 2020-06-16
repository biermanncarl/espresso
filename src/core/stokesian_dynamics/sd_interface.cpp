/*
 * Copyright (C) 2010-2020 The ESPResSo project
 *
 * This file is part of ESPResSo.
 *
 * ESPResSo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ESPResSo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.hpp"

#if defined(STOKESIAN_DYNAMICS) || defined(STOKESIAN_DYNAMICS_GPU)

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/config.hpp>

#include "ParticleRange.hpp"
#include "communication.hpp"
#include "errorhandling.hpp"
#include "integrate.hpp"
#include "particle_data.hpp"
#include "rotation.hpp"
#include "thermostat.hpp"

#include "serialization/SD_particle_data.hpp"

#include <utils/Vector.hpp>
#include <utils/mpi/gather_buffer.hpp>
#include <utils/mpi/scatter_buffer.hpp>

#ifdef STOKESIAN_DYNAMICS
#include "stokesian_dynamics/sd_cpu.hpp"
#endif

#ifdef STOKESIAN_DYNAMICS_GPU
#include "stokesian_dynamics/sd_gpu.hpp"
#endif

#include "sd_interface.hpp"

namespace {
double sd_viscosity = -1.0;

enum { CPU, GPU, INVALID } device = INVALID;

std::unordered_map<int, double> radius_dict;

double sd_kT = 0.0;

std::size_t sd_seed = 0UL;

int sd_flags = 0;

/** Buffer that holds local particle data, and all particles on the master node
 *  used for sending particle data to master node. */
std::vector<SD_particle_data> parts_buffer{};

/** Buffer that holds the (translational and angular) velocities of the local
 *  particles on each node, used for returning results. */
std::vector<double> v_sd{};

} // namespace

/** Pack selected properties of all local particles into a buffer. */
void sd_gather_local_particles(ParticleRange const &parts) {
  std::size_t n_part = parts.size();

  parts_buffer.resize(n_part);
  std::size_t i = 0;

  for (auto const &p : parts) {
    parts_buffer[i].id = p.p.is_virtual ? -1 : p.p.identity;
    parts_buffer[i].type = p.p.type;

    parts_buffer[i].pos[0] = p.r.p[0];
    parts_buffer[i].pos[1] = p.r.p[1];
    parts_buffer[i].pos[2] = p.r.p[2];

    // Velocity is not needed for SD, thus omit while gathering.
    // Particle orientation is also not needed for SD
    // since all particles are assumed spherical.

    parts_buffer[i].ext_force[0] = p.f.f[0];
    parts_buffer[i].ext_force[1] = p.f.f[1];
    parts_buffer[i].ext_force[2] = p.f.f[2];

    parts_buffer[i].ext_torque[0] = p.f.torque[0];
    parts_buffer[i].ext_torque[1] = p.f.torque[1];
    parts_buffer[i].ext_torque[2] = p.f.torque[2];

    // radius_dict is not initialized on slave nodes -> need to assign radius
    // later on master node
    parts_buffer[i].r = 0;

    i++;
  }
}

/** Update translational and rotational velocities of all particles. */
void sd_update_locally(ParticleRange const &parts) {
  std::size_t i = 0;

  // Even though on the master node, the v_sd vector is larger than
  // the (local) parts vector, this should still work. Because the local
  // particles correspond to the first 6*n entries in the master's v_sd
  // (which holds the velocities of ALL particles).

  for (auto &p : parts) {
    // skip virtual particles
    if (p.p.is_virtual) {
      continue;
    }

    // Copy velocities
    p.m.v[0] = v_sd[6 * i];
    p.m.v[1] = v_sd[6 * i + 1];
    p.m.v[2] = v_sd[6 * i + 2];

    p.m.omega[0] = v_sd[6 * i + 3];
    p.m.omega[1] = v_sd[6 * i + 4];
    p.m.omega[2] = v_sd[6 * i + 5];

    i++;
  }
}

void set_sd_viscosity(double eta) { sd_viscosity = eta; }

double get_sd_viscosity() { return sd_viscosity; }

void set_sd_device(std::string const &dev) {
  device = INVALID;
#ifdef STOKESIAN_DYNAMICS
  if (dev == "cpu") {
    device = CPU;
  }
#endif
#ifdef STOKESIAN_DYNAMICS_GPU
  if (dev == "gpu") {
    device = GPU;
  }
#endif
}

std::string get_sd_device() {
  switch (device) {
  case CPU:
    return "cpu";
  case GPU:
    return "gpu";
  default:
    return "invalid";
  }
}

void set_sd_radius_dict(std::unordered_map<int, double> const &x) {
  radius_dict = x;
}

std::unordered_map<int, double> get_sd_radius_dict() { return radius_dict; }

void set_sd_kT(double kT) { sd_kT = kT; }

double get_sd_kT() { return sd_kT; }

void set_sd_seed(std::size_t seed) { sd_seed = seed; }

std::size_t get_sd_seed() { return sd_seed; }

void set_sd_flags(int flg) { sd_flags = flg; }

int get_sd_flags() { return sd_flags; }

void propagate_vel_pos_sd(const ParticleRange &particles) {
  std::size_t n_part_local = particles.size();

  if (this_node == 0) {
    if (integ_switch & INTEG_METHOD_SD) {
      if (BOOST_UNLIKELY(sd_viscosity < 0.0)) {
        runtimeErrorMsg() << "sd_viscosity has an invalid value: " +
                                 std::to_string(sd_viscosity);
        return;
      }

      if (BOOST_UNLIKELY(sd_kT < 0.0)) {
        runtimeErrorMsg() << "sd_kT has an invalid value: " +
                                 std::to_string(sd_viscosity);
        return;
      }

      sd_gather_local_particles(particles);
      Utils::Mpi::gather_buffer(parts_buffer, comm_cart, 0);

      std::size_t n_part = parts_buffer.size();

      static std::vector<int> id{};
      static std::vector<double> x_host{};
      static std::vector<double> f_host{};
      static std::vector<double> a_host{};

      // n_part not expected to change, but anyway ...
      id.resize(n_part);
      x_host.resize(6 * n_part);
      f_host.resize(6 * n_part);
      a_host.resize(n_part);

      std::size_t i = 0;
      for (auto const &p : parts_buffer) {
        id[i] = parts_buffer[i].id;

        x_host[6 * i + 0] = p.pos[0];
        x_host[6 * i + 1] = p.pos[1];
        x_host[6 * i + 2] = p.pos[2];
        // Actual orientation is not needed, just need default.
        x_host[6 * i + 3] = 1;
        x_host[6 * i + 4] = 0;
        x_host[6 * i + 5] = 0;

        f_host[6 * i + 0] = p.ext_force[0];
        f_host[6 * i + 1] = p.ext_force[1];
        f_host[6 * i + 2] = p.ext_force[2];

        f_host[6 * i + 3] = p.ext_torque[0];
        f_host[6 * i + 4] = p.ext_torque[1];
        f_host[6 * i + 5] = p.ext_torque[2];

        double radius = radius_dict[p.type];

        if (BOOST_UNLIKELY(radius < 0.0)) {
          runtimeErrorMsg()
              << "particle of type " +
                     std::to_string(int(parts_buffer[i].type)) +
                     " has an invalid radius: " + std::to_string(radius);
          return;
        }

        a_host[i] = radius;

        ++i;
      }

      std::size_t offset = std::round(sim_time / time_step);
      switch (device) {

#ifdef STOKESIAN_DYNAMICS
      case CPU:
        v_sd = sd_cpu(x_host, f_host, a_host, n_part, sd_viscosity,
                      std::sqrt(sd_kT / time_step), offset, sd_seed, sd_flags);
        break;
#endif

#ifdef STOKESIAN_DYNAMICS_GPU
      case GPU:
        v_sd = sd_gpu(x_host, f_host, a_host, n_part, sd_viscosity,
                      std::sqrt(sd_kT / time_step), offset, sd_seed, sd_flags);
        break;
#endif

      default:
        runtimeErrorMsg()
            << "Invalid device for Stokesian dynamics. Available devices:"
#ifdef STOKESIAN_DYNAMICS
               " cpu"
#endif
#ifdef STOKESIAN_DYNAMICS_GPU
               " gpu"
#endif
            ;
        return;
      }

      Utils::Mpi::scatter_buffer(
          v_sd.data(), static_cast<int>(n_part_local * 6), comm_cart, 0);
      sd_update_locally(particles);
    }

  } else { // if (this_node == 0)

    if (thermo_switch & THERMO_SD) {
      sd_gather_local_particles(particles);
      Utils::Mpi::gather_buffer(parts_buffer, comm_cart, 0);

      v_sd.resize(n_part_local * 6);

      // now wait while master node is busy ...

      Utils::Mpi::scatter_buffer(
          v_sd.data(), static_cast<int>(n_part_local * 6), comm_cart, 0);
      sd_update_locally(particles);
    }

  } // if (this_node == 0) {...} else
}

#endif // STOKESIAN_DYNAMICS
