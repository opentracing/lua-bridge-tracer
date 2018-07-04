#pragma once

#include "lua_class_description.h"

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

  static const LuaClassDescription description;
 private:
   std::shared_ptr<opentracing::Span> span_;

  static int free(lua_State* L) noexcept;

  static int finish(lua_State* L) noexcept;
};
}  // namespace lua_bridge_tracer
