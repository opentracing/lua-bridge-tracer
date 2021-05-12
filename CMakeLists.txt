cmake_minimum_required(VERSION 3.1)

project(lua-bridge-tracer)

find_package(Lua 5.3)
find_package(OpenTracing 1.6.0)

set(CMAKE_CXX_STANDARD 11)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -Wl,-undefined,dynamic_lookup")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -Wl,--unresolved-symbols=ignore-in-object-files")
endif()

include_directories(SYSTEM ${LUA_INCLUDE_DIR})

add_library(opentracing_bridge_tracer SHARED src/module.cpp
                                             src/utility.cpp
                                             src/dynamic_tracer.cpp
                                             src/lua_tracer.cpp
                                             src/carrier.cpp
                                             src/lua_span_context.cpp
                                             src/lua_span.cpp)

target_link_libraries(opentracing_bridge_tracer OpenTracing::opentracing)
set_target_properties(opentracing_bridge_tracer PROPERTIES PREFIX "")
set_target_properties(opentracing_bridge_tracer PROPERTIES SUFFIX ".so")

set(LUA_MODULE_DIR ${CMAKE_INSTALL_PREFIX}/lib/lua/${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR})

install(TARGETS opentracing_bridge_tracer LIBRARY DESTINATION ${LUA_MODULE_DIR})
