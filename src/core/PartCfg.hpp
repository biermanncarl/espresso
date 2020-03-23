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
#ifndef CORE_PART_CFG_HPP
#define CORE_PART_CFG_HPP

#include "Particle.hpp"
#include "ParticleCache.hpp"
#include "cells.hpp"
#include "grid.hpp"

#include "serialization/Particle.hpp"

/**
 * @brief Proxy class that gets a particle range from #local_particles.
 */
class GetLocalParts {
public:
  auto operator()() const { return cell_structure.local_cells().particles(); }
};

/** Unfold coordinates to physical position. */
class PositionUnfolder {
public:
  template <typename Particle> void operator()(Particle &p) const {
    p.r.p += image_shift(p.l.i, box_geo.length());
    p.l.i = {};
  }
};

/** @brief Cache of particles */
using PartCfg = ParticleCache<GetLocalParts, PositionUnfolder>;
#endif
