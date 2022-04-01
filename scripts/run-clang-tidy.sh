#!/usr/bin/env bash

PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

clang-tidy \
    --config-file="${PROJECT_PATH}/.clang-tidy" \
    -p="${PROJECT_PATH}/build" \
    --header-filter="${CONDA_PREFIX}/include/llvm/**/*.h" \
    --extra-arg="-I${PROJECT_PATH}/arx/include" \
    $@
