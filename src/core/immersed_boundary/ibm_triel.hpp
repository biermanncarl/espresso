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

#ifndef IBM_TRIEL_H
#define IBM_TRIEL_H

#include "Particle.hpp"
#include "bonded_interactions/bonded_interaction_data.hpp"
#include "config.hpp"

#include <utils/Vector.hpp>

#include <boost/optional.hpp>

#include <tuple>

/** Set the IBM Triel parameters.
 *  Also calculate and store the reference state.
 */
int IBM_Triel_SetParams(int bond_type, int ind1, int ind2, int ind3,
                        double maxDist, tElasticLaw elasticLaw, double k1,
                        double k2);

/** Calculate the forces.
 *  The equations can be found in Appendix C of @cite kruger12a.
 *  @return the forces on @p p1, @p p2, @p p3
 */
boost::optional<std::tuple<Utils::Vector3d, Utils::Vector3d, Utils::Vector3d>>
IBM_Triel_CalcForce(Particle const &p1, Particle const &p2, Particle const &p3,
                    Bonded_ia_parameters const &iaparams);

#endif
