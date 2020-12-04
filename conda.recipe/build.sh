#!/bin/bash
set -eu

### Create Makefiles
CPPFLAGS="${CPPFLAGS} -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS"
cmake \
      -S src \
      -B build \
      ${CMAKE_ARGS} \
      -DCMAKE_PREFIX_PATH=$PREFIX \
      -DCMAKE_INSTALL_PREFIX=$PREFIX \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=$PREFIX/lib \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="$CFLAGS $CPPFLAGS" \
      -DCMAKE_CXX_FLAGS="$CXXFLAGS $CPPFLAGS -lrt" \
      -DENABLE_PYTHON=OFF \
      -DPython3_EXECUTABLE=${PYTHON} \
      -DUSE_SYSTEM_CIVET=ON \
      -DUSE_SYSTEM_CURL=ON \
      -DUSE_SYSTEM_CXXOPTS=ON \
      -DUSE_SYSTEM_GSLLITE=ON \
      -DUSE_SYSTEM_LIBARCHIVE=ON \
      -DUSE_SYSTEM_LIBRDKAFKA=ON \
      -DUSE_SYSTEM_LIBSODIUM=ON \
      -DUSE_SYSTEM_OPENSSL=ON \
      -DUSE_SYSTEM_OPEN62541=ON \
      -DUSE_SYSTEM_RAPIDJSON=ON \
      -DUSE_SYSTEM_SPDLOG=OFF \
      -DUSE_SYSTEM_SSH2=ON \
      -DUSE_SYSTEM_XML2=ON \
      -DUSE_SYSTEM_YAMLCPP=ON \
      -DUSE_SYSTEM_ZLIB=ON \
      -DBUILD_ROCKSDB=OFF \
      -DENABLE_AWS=OFF \
      -DENABLE_BUSTACHE=OFF \
      -DENABLE_LIBRDKAFKA=ON \
      -DENABLE_OPC=OFF \
      -DENABLE_OPENWSMAN=OFF \
      -DENABLE_PYTHON=ON \
      -DENABLE_SFTP=ON \
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
