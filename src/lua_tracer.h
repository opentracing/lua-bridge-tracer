#pragma once

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

  static int new_lua_tracer(lua_State* L) noexcept;

  static int free_lua_tracer(lua_State* L) noexcept;

  static int start_span(lua_State* L);

  static const struct luaL_Reg methods [];

 private:
  std::shared_ptr<opentracing::Tracer> tracer_;
};

}  // namespace lua_bridge_tracer
