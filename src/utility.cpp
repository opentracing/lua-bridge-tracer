#include "utility.h"

#include <stdexcept>

namespace lua_bridge_tracer {
std::chrono::system_clock::time_point convert_timestamp(lua_State* L, int index) {
  using SystemClock = std::chrono::system_clock;
  switch (lua_type(L, index)) {
    case LUA_TNUMBER:
      break;
    case LUA_TNIL:
    case LUA_TNONE:
      return {};
    default:
      throw std::runtime_error{"timestamp must be a number"};
  }
  auto time_since_epoch =
      std::chrono::microseconds{static_cast<uint64_t>(lua_tonumber(L, index))};
  return SystemClock::from_time_t(std::time_t(0)) +
         std::chrono::duration_cast<SystemClock::duration>(time_since_epoch);
}
}  // namespace lua_bridge_tracer
