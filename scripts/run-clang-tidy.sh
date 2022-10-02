#!/usr/bin/env bash

PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

set -ex

TMP_DIR=/tmp/arx
mkdir -p ${TMP_DIR}
cp ${PROJECT_PATH}/artifacts/compile_commands.json.tmpl ${TMP_DIR}/compile_commands.json

sed -i s:\{\{PROJECT_PATH\}\}:${PROJECT_PATH}:g ${TMP_DIR}/compile_commands.json
sed -i s:\{\{CONDA_PREFIX\}\}:${CONDA_PREFIX}:g ${TMP_DIR}/compile_commands.json

clang-tidy \
    --config-file="${PROJECT_PATH}/.clang-tidy" \
    --header-filter="${CONDA_PREFIX}/include" \
    --extra-arg="-I${PROJECT_PATH}/src" \
    -p=${TMP_DIR} \
    ${PROJECT_PATH}/src/*.cpp \
    ${PROJECT_PATH}/src/*.h \
    ${PROJECT_PATH}/src/codegen/*.cpp \
    ${PROJECT_PATH}/src/codegen/*.h
