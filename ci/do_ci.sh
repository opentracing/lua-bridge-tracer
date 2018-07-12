#!/bin/bash

set -e

function setup_asan_flags()
{
  export CFLAGS="$CFLAGS -Werror -fno-omit-frame-pointer -fsanitize=address"
  export CXXFLAGS="$CXXFLAGS -Werror -fno-omit-frame-pointer -fsanitize=address"
  export LDFLAGS="$LDFLAGS -Werror -fno-omit-frame-pointer -fsanitize=address"
}

export MOCKTRACER=/usr/local/lib/libopentracing_mocktracer.so

if [[ "$1" == "test" ]]; then
  setup_asan_flags
  ./ci/install_opentracing.sh
  ./ci/install_lua.sh
  ./ci/install_rocks.sh
  mkdir /build && cd /build
  cmake -DCMAKE_BUILD_TYPE=Debug /src
  make && make install
  ldconfig
  LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4 busted test/tracer.lua
  exit 0
fi
