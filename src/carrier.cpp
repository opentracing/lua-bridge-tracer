#include "carrier.h"

#include <iostream>

namespace lua_bridge_tracer {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
LuaCarrierWriter::LuaCarrierWriter(lua_State* lua_state) : lua_state_{lua_state} {}

//------------------------------------------------------------------------------
// Set
//------------------------------------------------------------------------------
opentracing::expected<void> LuaCarrierWriter::Set(
    opentracing::string_view key, opentracing::string_view value) const {
  // See https://stackoverflow.com/a/20148091/4447365
  lua_pushlstring(lua_state_, key.data(), key.size());
  lua_pushlstring(lua_state_, value.data(), value.size());
  lua_settable(lua_state_, -3);

  return {};
}
}  // namespace lua_bridge_tracer
