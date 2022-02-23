#!/usr/bin/env bash

mkdir -p build
cmake \
    -GNinja \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DCMAKE_C_COMPILER=${CC} \
    -DCMAKE_CXX_COMPILER=${CXX} \
    ..
cmake --build .
cmake --install . --config Release -v
