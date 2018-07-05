#include "lua_span.h"

#define METATABLE "lua_opentracing_bridge.span"

namespace lua_bridge_tracer {
//------------------------------------------------------------------------------
// check_lua_span
//------------------------------------------------------------------------------
static LuaSpan* check_lua_span(lua_State* L) noexcept {
  void* user_data = luaL_checkudata(L, 1, METATABLE);
  luaL_argcheck(L, user_data != NULL, 1, "`" METATABLE "' expected");
  return *static_cast<LuaSpan**>(user_data);
}

//------------------------------------------------------------------------------
// free
//------------------------------------------------------------------------------
int LuaSpan::free(lua_State* L) noexcept {
  auto span = check_lua_span(L);
  delete span;
  return 0;
}

//------------------------------------------------------------------------------
// finish
//------------------------------------------------------------------------------
int LuaSpan::finish(lua_State* L) noexcept {
  auto span = check_lua_span(L);
  span->span_->Finish();
  return 0;
}

//------------------------------------------------------------------------------
// description
//------------------------------------------------------------------------------
const LuaClassDescription LuaSpan::description = {
    nullptr,
    METATABLE,
    LuaSpan::free,
    {{nullptr, nullptr}},
    {{"finish", LuaSpan::finish}, {nullptr, nullptr}}};
}  // namespace lua_bridge_tracer
