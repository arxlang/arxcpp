#!/usr/bin/env bash

set -e

DEBUG=""
TMP_DIR=/tmp/arx
mkdir -p "${TMP_DIR}"

if [[ "${1}" == "--debug" ]]; then
  DEBUG="gdb --args"
fi

TEST_DIR_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

# load utils functions
. "${TEST_DIR_PATH}/scripts/utils.sh"

ARX="${DEBUG} ./build/arx"


for test_name in "fibonacci" "sum" "average"; do
  print_header "${test_name}"
  OBJECT_FILE="${TMP_DIR}/${test_name}"

  ${ARX} --output "${OBJECT_FILE}" --input "examples/${test_name}.arx"

  clang++ "${TEST_DIR_PATH}/main-objects/${test_name}.cpp" ${OBJECT_FILE} -o main
  ./main
done
