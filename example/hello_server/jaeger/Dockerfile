FROM ubuntu:eoan

ARG OPENTRACING_CPP_VERSION=v1.6.0
ARG JAEGER_VERSION=47fbf19aae2e48d59dd8335b6f2a1e1a99abba7f

COPY . /example

RUN set -x \
  && apt-get update \
  && apt-get install --no-install-recommends --no-install-suggests -y \
              build-essential \
              gettext \
              cmake \
              git \
              gnupg2 \
              software-properties-common \
              curl \
              ca-certificates \
              wget \
              lua5.1 lua5.1-dev \
              libyaml-cpp-dev \
### Build opentracing-cpp
  && cd / \
  && git clone -b $OPENTRACING_CPP_VERSION https://github.com/opentracing/opentracing-cpp.git \
  && cd opentracing-cpp \
  && mkdir .build && cd .build \
  && cmake -DBUILD_STATIC_LIBS=OFF -DBUILD_TESTING=OFF .. \
  && make && make install \
### Build bridge tracer
  && cd / \
  && git clone https://github.com/opentracing/lua-bridge-tracer.git \
  && cd lua-bridge-tracer \
  && mkdir .build && cd .build \
  && cmake .. \
  && make && make install \
### Install luvit
  && cd / \
  && curl -L https://github.com/luvit/lit/raw/3.6.0/get-lit.sh | sh \
### Install tracers
  && git clone https://github.com/jaegertracing/jaeger-client-cpp.git \
  && cd /jaeger-client-cpp \
  && git checkout ${JAEGER_VERSION} \
  && mkdir build && cd build \
  && cmake -DCMAKE_BUILD_TYPE=Release \
           -DBUILD_TESTING=OFF \
           -DJAEGERTRACING_WITH_YAML_CPP=ON .. \
  && make && make install \
  && ln -s /usr/local/lib/libjaegertracing.so /usr/local/lib/libjaegertracing_plugin.so \
  && cd / \
### Run ldconfig
  && ldconfig

EXPOSE 8080

ENTRYPOINT ["/luvit", "/example/server.lua"]
