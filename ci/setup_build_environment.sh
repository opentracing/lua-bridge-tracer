#!/bin/bash

set -e
apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
                ca-certificates \
                build-essential \
                cmake \
                pkg-config \
                git \
                automake \
                autogen \
                autoconf \
                libtool \
                ssh \
                wget \
                curl \
                unzip \
                libreadline6-dev
