/*
 * Copyright (C) 2010-2020 The ESPResSo project
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010
 *   Max-Planck-Institute for Polymer Research, Theory Group
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

#include "reduce_observable_stat.hpp"

#include <boost/mpi/collectives/reduce.hpp>

#include <functional>

/** Reduce contributions from all MPI ranks. */
boost::optional<Observable_stat> reduce(boost::mpi::communicator const &comm,
                                        Observable_stat const &obs) {
  if (comm.rank() == 0) {
    Observable_stat res{obs.chunk_size()};

    boost::mpi::reduce(comm, obs.data_().data(), obs.data_().size(),
                       res.data_().data(), std::plus<>{}, 0);

    return res;
  }

  boost::mpi::reduce(comm, obs.data_().data(), obs.data_().size(),
                     std::plus<>{}, 0);

  return {};
}
