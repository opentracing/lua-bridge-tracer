#include "lua_tracer.h"

#include "dynamic_tracer.h"
#include "lua_span.h"

#include <opentracing/dynamic_load.h>

#include <iostream>
#include <stdexcept>
#include <cstdint>

#define METATABLE "lua_opentracing_bridge.tracer"

namespace lua_bridge_tracer {
//------------------------------------------------------------------------------
// check_lua_tracer
//------------------------------------------------------------------------------
static LuaTracer* check_lua_tracer(lua_State* L) noexcept {
  void* user_data = luaL_checkudata(L, 1, METATABLE);
  luaL_argcheck(L, user_data != NULL, 1, "`" METATABLE "' expected");
  return *static_cast<LuaTracer**>(user_data);
}

//------------------------------------------------------------------------------
// compute_start_time
//------------------------------------------------------------------------------
static std::chrono::system_clock::time_point compute_start_time(lua_State* L) {
  using SystemClock = std::chrono::system_clock;
  switch (lua_type(L, -1)) {
    case LUA_TNUMBER:
      break;
    case LUA_TNIL:
    case LUA_TNONE:
      return {};
    default:
      throw std::runtime_error{"start_time must be a number"};
  }
  auto time_since_epoch =
      std::chrono::microseconds{static_cast<uint64_t>(lua_tonumber(L, -1))};
  return SystemClock::from_time_t(std::time_t(0)) +
         std::chrono::duration_cast<SystemClock::duration>(time_since_epoch);
}

//------------------------------------------------------------------------------
// compute_start_span_options
//------------------------------------------------------------------------------
static opentracing::StartSpanOptions compute_start_span_options(lua_State* L) {
  auto top = lua_gettop(L);
  opentracing::StartSpanOptions result;
  if (top < 3) {
    return result;
  }
  if (top > 3) {
    throw std::runtime_error{"too many arguments"};
  }

  lua_getfield(L, -1, "start_time");
  result.start_system_timestamp = compute_start_time(L);
  lua_pop(L, 1);

  return result;
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
  auto userdata = static_cast<LuaSpan**>(lua_newuserdata(L, sizeof(LuaSpan*)));
  auto top = lua_gettop(L);
  if (top >= 3) {
    luaL_checktype(L, 3, LUA_TTABLE);
  }

  try {
    auto start_span_options = compute_start_span_options(L);
    auto span = tracer->tracer_->StartSpanWithOptions(operation_name,
                                                      start_span_options);
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
// close
//------------------------------------------------------------------------------
int LuaTracer::close(lua_State* L) noexcept {
  auto tracer = check_lua_tracer(L);
  tracer->tracer_->Close();
  return 0;
}

//------------------------------------------------------------------------------
// description
//------------------------------------------------------------------------------
const LuaClassDescription LuaTracer::description = {
    "bridge_tracer",
    METATABLE,
    LuaTracer::free,
    {{"new", LuaTracer::new_lua_tracer}, {nullptr, nullptr}},
    {{"start_span", LuaTracer::start_span},
     {"close", LuaTracer::close},
     {nullptr, nullptr}}};
}  // namespace lua_bridge_tracer
