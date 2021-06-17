/*
 * Copyright (C) 2021 The ESPResSo project
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

#ifndef SCRIPT_INTERFACE_INTERACTIONS_BONDED_INTERACTIONS_HPP
#define SCRIPT_INTERFACE_INTERACTIONS_BONDED_INTERACTIONS_HPP

#include "BondedInteraction.hpp"

#include "core/bonded_interactions/bonded_interaction_data.hpp"

#include "script_interface/ObjectMap.hpp"
#include "script_interface/ScriptInterface.hpp"

#include <memory>
#include <unordered_map>

    extern int this_node;
namespace ScriptInterface {
namespace Interactions {
class BondedInteractions : public ObjectMap<BondedInteraction> {
  using container_type = std::unordered_map<int, std::shared_ptr<BondedInteraction>>;
public:
  using key_type = typename container_type::key_type;
  using mapped_type = typename container_type::mapped_type;

  key_type insert_in_core(mapped_type const &obj_ptr) override {
    return ::bonded_ia_params.insert(obj_ptr->bonded_ia());
  }
  void insert_in_core(key_type const &key, mapped_type const &obj_ptr) override {
    ::bonded_ia_params.insert(key, obj_ptr->bonded_ia());
  }
  void erase_in_core(key_type const &key) override {
    ::bonded_ia_params.erase(key);
  }
};
} // namespace Interactions
} // namespace ScriptInterface

#endif
