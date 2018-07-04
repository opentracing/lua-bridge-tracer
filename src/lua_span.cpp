#include "lua_span.h"

#define CLASS_NAME "bridge_tracer"

namespace lua_bridge_tracer {
//------------------------------------------------------------------------------
// check_lua_span
//------------------------------------------------------------------------------
static LuaSpan* check_lua_span(lua_State* L) noexcept {
  void* user_data = luaL_checkudata(L, 1, LuaSpan::metatable);
  luaL_argcheck(L, user_data != NULL, 1, "`" CLASS_NAME "' expected");
  return *static_cast<LuaSpan**>(user_data);
}

//------------------------------------------------------------------------------
// free
//------------------------------------------------------------------------------
int LuaSpan::free(lua_State* L) noexcept {
  auto tracer = static_cast<LuaSpan**>(lua_touserdata(L, 1));
  delete *tracer;
  return 0;
}

//------------------------------------------------------------------------------
// finish
//------------------------------------------------------------------------------
int LuaSpan::finish(lua_State* L) noexcept {
  return 0;
}

//------------------------------------------------------------------------------
// name
//------------------------------------------------------------------------------
const char* LuaSpan::name = CLASS_NAME;

//------------------------------------------------------------------------------
// metatable
//------------------------------------------------------------------------------
const char* LuaSpan::metatable = "lua_opentracing_bridge.span";

//------------------------------------------------------------------------------
// methods
//------------------------------------------------------------------------------
const struct luaL_Reg LuaSpan::methods[] = {
    {"finish", LuaSpan::finish},
    {nullptr, nullptr}};
}  // namespace lua_bridge_tracer
