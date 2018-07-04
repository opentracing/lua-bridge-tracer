#include "lua_tracer.h"

#include "lua_span.h"
#include "dynamic_tracer.h"

#include <opentracing/dynamic_load.h>

#include <iostream>
#include <stdexcept>

#define CLASS_NAME "bridge_tracer"

namespace lua_bridge_tracer {
//------------------------------------------------------------------------------
// check_lua_tracer
//------------------------------------------------------------------------------
static LuaTracer* check_lua_tracer(lua_State* L) {
  void* user_data = luaL_checkudata(L, 1, LuaTracer::description.metatable);
  luaL_argcheck(L, user_data != NULL, 1, "`" CLASS_NAME "' expected");
  return *static_cast<LuaTracer**>(user_data);
}

//------------------------------------------------------------------------------
// new_lua_tracer
//------------------------------------------------------------------------------
int LuaTracer::new_lua_tracer(lua_State* L) noexcept {
  auto library_name = luaL_checkstring(L, -2);
  auto config = luaL_checkstring(L, -1);
  auto userdata =
      static_cast<LuaTracer**>(lua_newuserdata(L, sizeof(LuaTracer*)));

  try {
    auto tracer = std::unique_ptr<LuaTracer>{
        new LuaTracer{load_tracer(library_name, config)}};
    *userdata = tracer.release();

    // tag the metatable
    luaL_getmetatable(L, description.metatable);
    lua_setmetatable(L, -2);

    return 1;
  } catch (const std::exception& e) {
    lua_pushstring(L, e.what());
  }
  return lua_error(L);
}

//------------------------------------------------------------------------------
// free
//------------------------------------------------------------------------------
int LuaTracer::free(lua_State* L) noexcept {
  auto tracer = check_lua_tracer(L);
  delete tracer;
  return 0;
}

//------------------------------------------------------------------------------
// start_span
//------------------------------------------------------------------------------
int LuaTracer::start_span(lua_State* L) noexcept { 
  auto tracer = check_lua_tracer(L);
  auto operation_name = luaL_checkstring(L, 2);
  auto userdata =
      static_cast<LuaSpan**>(lua_newuserdata(L, sizeof(LuaSpan*)));

  try {
    auto span = tracer->tracer_->StartSpan(operation_name);
    if (span == nullptr) {
      throw std::runtime_error{"unable to create span"};
    }
    auto lua_span = std::unique_ptr<LuaSpan>{
      new LuaSpan{std::shared_ptr<opentracing::Span>{span.release()}}};
    *userdata = lua_span.release();

    luaL_getmetatable(L, LuaSpan::description.metatable);
    lua_setmetatable(L, -2);

    return 1;
  } catch (const std::exception& e) {
    lua_pushstring(L, e.what());
  }
  return lua_error(L);
}

//------------------------------------------------------------------------------
// description
//------------------------------------------------------------------------------
const LuaClassDescription LuaTracer::description = {
  CLASS_NAME,
  "lua_opentracing_bridge.tracer",
  LuaTracer::free,
  {
    {"new", LuaTracer::new_lua_tracer},
    {nullptr, nullptr}
  },
  {
    {"start_span", LuaTracer::start_span},
    {nullptr, nullptr}
  }
};
}  // namespace lua_bridge_tracer
