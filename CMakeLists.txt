cmake_minimum_required(VERSION 3.1)

project(lua-bridge-tracer)

find_package(Lua 5.1)
find_package(OpenTracing 1.4.0)

set(CMAKE_CXX_STANDARD 11)

include_directories(SYSTEM ${LUA_INCLUDE_DIR})

add_library(opentracing_bridge_tracer SHARED src/module.cpp
                                             src/dynamic_tracer.cpp
                                             src/lua_tracer.cpp
                                             src/carrier.cpp
                                             src/lua_span_context.cpp
                                             src/lua_span.cpp)
target_link_libraries(opentracing_bridge_tracer ${LUA_LIBRARIES}
                                                OpenTracing::opentracing)
set_target_properties(opentracing_bridge_tracer PROPERTIES PREFIX "")
set_target_properties(opentracing_bridge_tracer PROPERTIES SUFFIX ".so")
