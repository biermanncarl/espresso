/*
 * Copyright (C) 2020 The ESPResSo project
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
#ifndef SCRIPT_INTERFACE_LOCAL_CONTEXT_HPP
#define SCRIPT_INTERFACE_LOCAL_CONTEXT_HPP

#include "Context.hpp"
#include "ObjectHandle.hpp"

#include <utils/Factory.hpp>

#include <memory>
#include <string>
#include <utility>

namespace ScriptInterface {

/**
 * @brief Trivial context.
 *
 * This context just maintains a local copy of an
 * object.
 */
class LocalContext : public Context {
  Utils::Factory<ObjectHandle> m_factory;

public:
  explicit LocalContext(Utils::Factory<ObjectHandle> factory)
      : m_factory(std::move(factory)) {}

  const Utils::Factory<ObjectHandle> &factory() const { return m_factory; }

  void notify_call_method(const ObjectHandle *, std::string const &,
                          VariantMap const &) override {}
  void notify_set_parameter(const ObjectHandle *, std::string const &,
                            Variant const &) override {}

  std::shared_ptr<ObjectHandle>
  make_shared(std::string const &name, const VariantMap &parameters,
              std::string const &internal_state = {}) override {
    auto sp = m_factory.make(name);
    set_context(sp.get());

    sp->construct(parameters);
    if (!internal_state.empty()) {
      sp->set_internal_state(internal_state);
    }

    return sp;
  }

  boost::string_ref name(const ObjectHandle *o) const override {
    assert(o);

    return factory().type_name(*o);
  }
};
} // namespace ScriptInterface

#endif
