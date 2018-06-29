#include <iostream>
#include <opentracing/dynamic_load.h>

extern "C" {
#include <lua.h>
#include <lua/lauxlib.h>
} // extern "C"

static int load_tracer(lua_State* L) {
  if (lua_gettop(L) < 2) {
    return luaL_error(L, "load_tracer takes two arguments");
  }
  if (lua_type(L, -2) != LUA_TSTRING) {
    return luaL_error(L, "tracer_library must be a string");
  }
  if (lua_type(L, -1) != LUA_TSTRING) {
    return luaL_error(L, "tracer_config must be a string");
  }
  auto library_name = lua_tostring(L, -2);
  auto config = lua_tostring(L, -1);

  std::string error_message;
  auto handle_maybe =
      opentracing::DynamicallyLoadTracingLibrary(library_name, error_message);
  if (!handle_maybe) {
    return luaL_error(L, error_message.c_str());
  }
  auto& handle = *handle_maybe;
  auto tracer_maybe = handle.tracer_factory().MakeTracer(config, error_message);
  if (!tracer_maybe) {
    return luaL_error(L, error_message.c_str());
  }

  std::cout << "Creating a tracer...\n";
  return 0;
}

extern "C" int luaopen_opentracing_bridge_tracer(lua_State* L) {
  lua_register(L, "load_tracer", load_tracer);
  return 0;
}
