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

#define BOOST_TEST_MODULE ObjectMap test
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "script_interface/LocalContext.hpp"
#include "script_interface/ObjectMap.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>

using namespace ScriptInterface;

struct ObjectMapImpl : ObjectMap<ObjectHandle> {
  using KeyType = int;
  std::unordered_map<KeyType, ObjectRef> mock_core;

private:
  void insert_in_core(KeyType const &key, ObjectRef const &obj_ptr) override {
    next_key = std::max(next_key, key + 1);
    mock_core[key] = obj_ptr;
  }
  KeyType insert_in_core(ObjectRef const &obj_ptr) override {
    auto const key = next_key++;
    mock_core[key] = obj_ptr;
    return key;
  }
  void erase_in_core(KeyType const &key) override { mock_core.erase(key); }
  KeyType next_key = static_cast<KeyType>(0);
};

BOOST_AUTO_TEST_CASE(default_construction) {
  // A defaulted ObjectMap has no elements.
  BOOST_CHECK(ObjectMapImpl{}.elements().empty());
}

BOOST_AUTO_TEST_CASE(inserting_elements) {
  ObjectMapImpl map;
  auto const first_key = 1;
  auto first_obj = ObjectRef{};
  map.insert(first_key, first_obj);
  BOOST_CHECK(map.elements().at(first_key) == first_obj);
  BOOST_CHECK(map.mock_core.at(first_key) == first_obj);
  auto second_obj = ObjectRef{};
  auto const second_key = map.insert(second_obj);
  BOOST_CHECK(second_key != first_key);
  BOOST_CHECK(map.elements().at(second_key) == second_obj);
  BOOST_CHECK(map.mock_core.at(second_key) == second_obj);
}

BOOST_AUTO_TEST_CASE(erasing_elements) {
  // An element that is removed from the map is
  // no longer an element of the map.
  auto e = ObjectRef{};
  ObjectMapImpl map;
  auto const key = map.insert(e);
  map.erase(key);
  BOOST_CHECK(map.elements().count(key) == 0);
  // And is removed from the core
  BOOST_CHECK(map.mock_core.count(key) == 0);
}

BOOST_AUTO_TEST_CASE(clearing_elements) {
  // A cleared map is empty.
  ObjectMapImpl map;
  map.insert(ObjectRef{});
  map.insert(ObjectRef{});
  map.clear();
  BOOST_CHECK(map.elements().empty());
  BOOST_CHECK(map.mock_core.empty());
}

BOOST_AUTO_TEST_CASE(serialization) {
  // In a context
  Utils::Factory<ObjectHandle> f;
  f.register_new<ObjectHandle>("ObjectHandle");
  f.register_new<ObjectMapImpl>("ObjectMap");
  auto ctx = std::make_shared<LocalContext>(f);
  // A list of some elements
  auto map = std::dynamic_pointer_cast<ObjectMapImpl>(
      ctx->make_shared("ObjectMap", {}));
  // with a bunch of elements

  map->insert(1, ctx->make_shared("ObjectHandle", {}));
  map->insert(2, ctx->make_shared("ObjectHandle", {}));
  // can be serialized to a string
  auto const s = map->serialize();
  // and can be restored to a list with the same elements
  auto const map2 = std::dynamic_pointer_cast<ObjectMapImpl>(
      ObjectHandle::deserialize(s, *ctx));
  BOOST_CHECK(map2->elements().size() == map->elements().size());
  BOOST_CHECK(map2->elements().at(1)->name() == "ObjectHandle");
  BOOST_CHECK(map2->elements().at(2)->name() == "ObjectHandle");
  // and the elements are restored to the core
  BOOST_CHECK(map2->mock_core.size() == 2);
  BOOST_CHECK(map2->mock_core.at(1)->name() == "ObjectHandle");
  BOOST_CHECK(map2->mock_core.at(2)->name() == "ObjectHandle");
  // key bookkeeping is also restored
  auto const key = map->insert(ctx->make_shared("ObjectHandle", {}));
  BOOST_CHECK(key == 3);
}
