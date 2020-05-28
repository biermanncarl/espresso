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
#ifndef OBSERVABLES_TotalForce_HPP
#define OBSERVABLES_TotalForce_HPP

#include "PidObservable.hpp"

#include <vector>

namespace Observables {
class TotalForce : public PidObservable {
public:
  using PidObservable::PidObservable;
  std::vector<size_t> shape() const override { return {3}; }

  std::vector<double>
  evaluate(ParticleReferenceRange particles,
           const ParticleObservables::traits<Particle> &traits) const override {
    std::vector<double> res(n_values());
    for (auto const &p : particles) {
      if (p.get().p.is_virtual)
        continue;
      res[0] += p.get().f.f[0];
      res[1] += p.get().f.f[1];
      res[2] += p.get().f.f[2];
    }
    return res;
  }
};
} // Namespace Observables
#endif
