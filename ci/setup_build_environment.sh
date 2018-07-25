#!/bin/bash

set -e
apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
                ca-certificates \
                build-essential \
                software-properties-common \
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
                libreadline6-dev \
                libncurses5-dev \
                gcc-8 \
                g++-8 \
                python python-setuptools python-pip

### Use gcc-8 (the default gcc has this problem when using with address sanitizer:
### https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84428)
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 800 --slave /usr/bin/g++ g++ /usr/bin/g++-8

pip install gcovr
