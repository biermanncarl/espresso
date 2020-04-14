/*
 * Copyright (C) 2010-2019 The ESPResSo project
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
#pragma once

#include "ParticleRange.hpp"
#include "config.hpp"
#include "particle_data.hpp"
#include "rotation.hpp"
#include "stokesian_dynamics/sd_interface.hpp"

#ifdef STOKESIAN_DYNAMICS

inline void
stokesian_dynamics_propagate_vel_pos(const ParticleRange &particles) {
  auto const skin2 = Utils::sqr(0.5 * skin);

  // Compute new (translational and rotational) velocities
  propagate_vel_pos_sd(particles);

  for (auto &p : particles) {
    // Translate particle
    p.r.p[0] += p.m.v[0] * time_step;
    p.r.p[1] += p.m.v[1] * time_step;
    p.r.p[2] += p.m.v[2] * time_step;

#ifdef ROTATION
    // Perform rotation
    local_rotate_particle(p, p.m.omega.normalize(),
                          p.m.omega.norm() * time_step);
#endif

    // Verlet criterion check
    if (Utils::sqr(p.r.p[0] - p.l.p_old[0]) +
            Utils::sqr(p.r.p[1] - p.l.p_old[1]) +
            Utils::sqr(p.r.p[2] - p.l.p_old[2]) >
        skin2)
      cell_structure.set_resort_particles(Cells::RESORT_LOCAL);
  }
}

inline void stokesian_dynamics_step_1(const ParticleRange &particles) {
  stokesian_dynamics_propagate_vel_pos(particles);
  sim_time += time_step;
}

inline void stokesian_dynamics_step_2(const ParticleRange &particles) {}

#endif // STOKESIAN_DYNAMICS
