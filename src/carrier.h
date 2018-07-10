#pragma once

#include <opentracing/propagation.h>

extern "C" {
#include <lua.h>
#include <lua/lauxlib.h>
} // extern "C"

namespace lua_bridge_tracer {
class LuaCarrierWriter : public opentracing::HTTPHeadersWriter {
 public:
   explicit LuaCarrierWriter(lua_State* lua_state);

  opentracing::expected<void> Set(
      opentracing::string_view key,
      opentracing::string_view value) const override;

 private:
  lua_State* lua_state_;
};
}  // namespace lua_bridge_tracer
