#!/usr/bin/env bash

PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

set -ex

mkdir -p /tmp/arx
cp ${PROJECT_PATH}/artifacts/compile_commands.json.tmpl /tmp/arx/compile_commands.json

sed -i s:\{\{PROJECT_PATH\}\}:${PROJECT_PATH}:g /tmp/arx/compile_commands.json
sed -i s:\{\{CONDA_PREFIX\}\}:${CONDA_PREFIX}:g /tmp/arx/compile_commands.json

clang-tidy \
    --config-file="${PROJECT_PATH}/.clang-tidy" \
    --header-filter="${CONDA_PREFIX}/include" \
    --extra-arg="-I${PROJECT_PATH}/arx/include" \
    -p=/tmp/arx \
    ${PROJECT_PATH}/arx/**/*.cpp \
    ${PROJECT_PATH}/arx/**/*.h
