#pragma once

#include <opentracing/tracer.h>

extern "C" {
#include <lua.h>
#include <lua/lauxlib.h>
} // extern "C"

namespace lua_bridge_tracer {
class LuaSpan {
 public:
   explicit LuaSpan(const std::shared_ptr<opentracing::Span>& span)
     : span_{span}
   {}

  static int free(lua_State* L) noexcept;

  static int finish(lua_State* L) noexcept;

  static const char* name;

  static const char* metatable;

  static const struct luaL_Reg methods [];
 private:
   std::shared_ptr<opentracing::Span> span_;
};
}  // namespace lua_bridge_tracer
