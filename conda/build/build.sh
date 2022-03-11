#!/usr/bin/env bash

CXX=clang++
CC=clang

mkdir -p ${SRC_DIR}/build
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

mkdir -p ${PREFIX}/bin/
mv ./arx ${PREFIX}/bin/
chmod +x ${PREFIX}/bin/arx
