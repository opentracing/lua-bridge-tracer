#pragma once

#include <chrono>

extern "C" {
#include <lua.h>
#include <lua/lauxlib.h>
} // extern "C"

namespace lua_bridge_tracer {
std::chrono::system_clock::time_point convert_timestamp(lua_State* L, int index);
}  // namespace lua_bridge_tracer
