#!/bin/bash
set -eu

### Create Makefiles
cmake -G Ninja \
      -S src \
      -B build \
      ${CMAKE_ARGS} \
      -DCMAKE_PREFIX_PATH=$PREFIX \
      -DCMAKE_INSTALL_PREFIX=$PREFIX \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=$PREFIX/lib \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="$CFLAGS $CPPFLAGS" \
      -DCMAKE_CXX_FLAGS="$CXXFLAGS $CPPFLAGS -lrt -lpthread" \
      -DUSE_SHARED_LIBS=OFF \
      -DBUILD_SHARED_LIBS=OFF \
      -DPORTABLE=ON \
      -DOPENSSL_OFF=OFF \
      -DBUILD_OPENSSL=OFF \
      -DUSE_SYSTEM_BZIP2=ON \
      -DUSE_SYSTEM_LIBSSH2=ON \
      -DUSE_SYSTEM_ZLIB=ON \
      -DENABLE_PYTHON=OFF \
      -DPython3_EXECUTABLE=${PYTHON} \
      -DDISABLE_CIVET=OFF \
      -DBUILD_CIVET=OFF \
      -DDISABLE_CURL=OFF \
      -DBUILD_CURL=OFF \
      -DDISABLE_EXPRESSION_LANGUAGE=ON \
      -DDISABLE_ROCKSDB=ON \
      -DDISABLE_SCRIPTING=ON  \
      -DSKIP_TESTS=OFF
   

## Build
cmake --build build -- -j${CPU_COUNT}

## Test
#cmake --build build -- test

### Install
#cmake --build build -- package

### Checking requires a recompile with DEBUG enabled
# cmake --build build -- check

### Copy the tools to $PREFIX/bin
# TODO: I someone needs the tools, please open a PR/issue.
# cp build/tools/{ldb,rocksdb_{dump,undump},sst_dump} $PREFIX/bin
