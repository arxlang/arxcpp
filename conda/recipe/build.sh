#!/usr/bin/env bash
set -ex

CXX=clang++
CC=clang

LDFLAGS="$(llvm-config --ldflags) $LDFLAGS"
CFLAGS="$(llvm-config --cflags) $CFLAGS"
CXXFLAGS="$(llvm-config --cxxflags) $CXXFLAGS"
CPPFLAGS="$(llvm-config --cppflags) $CPPFLAGS"

export CXXFLAGS="`echo $CXXFLAGS | sed 's/-fno-exceptions//'`"

mkdir -p ${SRC_DIR}/build/bin
cd ${SRC_DIR}/build

cmake \
    -GNinja \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DCMAKE_BUILD_TYPE=release \
    -DCMAKE_C_COMPILER=${CC} \
    -DCMAKE_CXX_COMPILER=${CXX} \
    ..
cmake --build .
cmake --install . -v

mv ${PREFIX}/build/bin ${PREFIX}/bin
chmod +x ${PREFIX}/bin/arx

ctest

set +x
