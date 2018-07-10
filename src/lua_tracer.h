#pragma once

#include "lua_class_description.h"

#include <opentracing/tracer.h>

extern "C" {
#include <lua.h>
#include <lua/lauxlib.h>
} // extern "C"

namespace lua_bridge_tracer {
class LuaTracer {
 public:
   explicit LuaTracer(const std::shared_ptr<opentracing::Tracer>& tracer)
     : tracer_{tracer}
   {}

  static const LuaClassDescription description;
 private:
  std::shared_ptr<opentracing::Tracer> tracer_;

  static int new_lua_tracer(lua_State* L) noexcept;

  static int free(lua_State* L) noexcept;

  static int start_span(lua_State* L) noexcept;

  template <class Carrier>
  static int inject(lua_State* L) noexcept;

  static int binary_inject(lua_State* L) noexcept;

  static int close(lua_State* L) noexcept;
};
}  // namespace lua_bridge_tracer
