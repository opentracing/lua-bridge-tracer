#include "lua_tracer.h"
#include "lua_span.h"

#include <iostream>
#include <opentracing/dynamic_load.h>

extern "C" {
#include <lua.h>
#include <lua/lauxlib.h>
} // extern "C"

static void setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup+1, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    lua_pushstring(L, l->name);
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -(nup+1));
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_settable(L, -(nup + 3));
  }
  lua_pop(L, nup);  /* remove upvalues */
}

static void make_lua_class(lua_State* L, const char* name,
                           const char* metatable, const luaL_Reg* methods,
                           int (*free)(lua_State*)) {
  luaL_newmetatable(L, metatable);

  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, free);
  lua_settable(L, -3);

  lua_newtable(L);
  setfuncs(L, methods, 0);
  lua_setglobal(L, name);
}

extern "C" int luaopen_opentracing_bridge_tracer(lua_State* L) {
  make_lua_class(L, lua_bridge_tracer::LuaTracer::name,
                 lua_bridge_tracer::LuaTracer::metatable,
                 lua_bridge_tracer::LuaTracer::methods,
                 lua_bridge_tracer::LuaTracer::free);

  make_lua_class(L, 
      lua_bridge_tracer::LuaSpan::name,
      lua_bridge_tracer::LuaSpan::metatable,
      lua_bridge_tracer::LuaSpan::methods,
      lua_bridge_tracer::LuaSpan::free);

  return 0;
}
