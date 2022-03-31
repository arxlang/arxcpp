#!/usr/bin/env bash

clang-tidy \
    --config-file .clang-tidy \
    arx/**/*.cpp \
    arx/**/*.h \
    -p build \
    --header-filter="^(?!${CONDA_PREFIX}.)*$|arx/include/*.h" \
    -- -I${CONDA_PREFIX}/include
