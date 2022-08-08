#!/usr/bin/env bash

PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

set -ex

clang-tidy \
    --config-file="${PROJECT_PATH}/.clang-tidy" \
    --header-filter="${CONDA_PREFIX}/include" \
    --extra-arg="-I${PROJECT_PATH}/arx/include" \
    -p=${PROJECT_PATH}/artifacts \
    ${PROJECT_PATH}/arx/**/*.cpp \
    ${PROJECT_PATH}/arx/**/*.h
