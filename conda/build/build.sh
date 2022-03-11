#!/usr/bin/env bash

CXX=clang++
CC=clang

mkdir -p build
cd build

cmake \
    -GNinja \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DCMAKE_C_COMPILER=${CC} \
    -DCMAKE_CXX_COMPILER=${CXX} \
    ..
cmake --build .
cmake --install . --config Release -v

mkdir -p ${PREFIX}/bin/
mv ./arx ${PREFIX}/bin/
chmod +x ${PREFIX}/bin/arx
