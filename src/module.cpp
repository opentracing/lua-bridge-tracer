#include "lua_tracer.h"

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

extern "C" int luaopen_opentracing_bridge_tracer(lua_State* L) {
  luaL_newmetatable(L, "lua_opentracing_bridge.tracer");

  /* set its __gc field */
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, lua_bridge_tracer::LuaTracer::free_lua_tracer);
  lua_settable(L, -3);

  lua_newtable(L);
  setfuncs(L, lua_bridge_tracer::LuaTracer::methods, 0);
  lua_setglobal(L, "bridge_tracer");
  return 1;
}
