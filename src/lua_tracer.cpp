#include "lua_tracer.h"
#include "dynamic_tracer.h"

#include <opentracing/dynamic_load.h>

#include <iostream>
#include <stdexcept>

namespace lua_bridge_tracer {
//------------------------------------------------------------------------------
// load_tracer
//------------------------------------------------------------------------------
static std::shared_ptr<opentracing::Tracer> load_tracer(
    const char* library_name, const char* config) {
  std::string error_message;
  auto handle_maybe =
      opentracing::DynamicallyLoadTracingLibrary(library_name, error_message);
  if (!handle_maybe) {
    throw std::runtime_error{error_message};
  }
  auto& handle = *handle_maybe;
  auto tracer_maybe = handle.tracer_factory().MakeTracer(config, error_message);
  if (!tracer_maybe) {
    throw std::runtime_error{error_message};
  }
  return *tracer_maybe;
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
    luaL_getmetatable(L, "lua_opentracing_bridge.tracer");
    lua_setmetatable(L, -2);

    return 1;
  } catch (const std::exception& e) {
    lua_pushstring(L, e.what());
  }
  return lua_error(L);
}

//------------------------------------------------------------------------------
// free_lua_tracer
//------------------------------------------------------------------------------
int LuaTracer::free_lua_tracer(lua_State* L) noexcept {
  auto tracer = static_cast<LuaTracer**>(lua_touserdata(L, 1));
  delete tracer;
  return 0;
}

//------------------------------------------------------------------------------
// start_span
//------------------------------------------------------------------------------
int LuaTracer::start_span(lua_State* L) { return 0; }

//------------------------------------------------------------------------------
// methods
//------------------------------------------------------------------------------
const struct luaL_Reg LuaTracer::methods[] = {
    {"new", LuaTracer::new_lua_tracer},
    {"start_span", LuaTracer::start_span},
    {nullptr, nullptr}};
}  // namespace lua_bridge_tracer
