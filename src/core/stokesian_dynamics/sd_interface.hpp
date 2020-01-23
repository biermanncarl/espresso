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

#ifndef STOKESIAN_DYNAMICS_INTERFACE_H
#define STOKESIAN_DYNAMICS_INTERFACE_H

#include "config.hpp"
#include "ParticleRange.hpp"

#include <string>
#include <unordered_map>
#include <vector>

#ifdef STOKESIAN_DYNAMICS

/* type for particle data transfer between nodes */
struct SD_particle_data {
  int on_node = -1;
  int id = -1;
  int type = 0; 
  
  /* particle radius */
  double r = -1;   // Where do I get this?  Particle type table?

  /* particle position */
  Utils::Vector3d pos = {0., 0., 0.};
  
  /* particle velocity */
  Utils::Vector3d vel = {0., 0., 0.};
  
  /* particle rotational velocity */
  Utils::Vector3d omega = {0., 0., 0.};
  
  /* quaternion to define particle orientation */
  Utils::Vector4d quat = {1., 0., 0., 0.};  // TODO: make sure rotations work as they should
  
  /* external force */
  Utils::Vector3d ext_force = {0.0, 0.0, 0.0};  // double or float?
  
  /* external torque */
  Utils::Vector3d ext_torque = {0.0, 0.0, 0.0};
  
};

/** gathers SD relevant information of local particles into a buffer */
std::vector<SD_particle_data> &sd_gather_local_particles(ParticleRange const &parts);

/** returns a reference to the local (translational and angular) velocities 
    buffer, which is guaranteed to have the appropriate size */
std::vector<double> &get_sd_local_v_buffer();

/** Performs the integration step locally on each node */
void sd_update_locally(ParticleRange const &parts); 

void set_sd_viscosity(double eta);
double get_sd_viscosity();

void set_sd_device(std::string const &dev);
std::string get_sd_device();

void set_sd_radius_dict(std::unordered_map<int, double> const &x);
std::unordered_map<int, double> get_sd_radius_dict();

void set_sd_kT(double kT);
double get_sd_kT();

void set_sd_flags(int flg);
int get_sd_flags();

void set_sd_seed(std::size_t seed);
std::size_t get_sd_seed();

void propagate_vel_pos_sd();

#endif // STOKESIAN_DYNAMICS

#endif // STOKESIAN_DYNAMICS_INTERFACE_H
