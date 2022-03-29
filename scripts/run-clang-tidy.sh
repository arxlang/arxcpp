#!/usr/bin/env bash

clang-tidy \
    --config-file .clang-tidy.yaml \
    arx/**/*.cpp \
    arx/**/*.h \
    -- \
    -I${CONDA_PREFIX}/include
