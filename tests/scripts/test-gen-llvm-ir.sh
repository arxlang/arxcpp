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
MAIN_EXE="${TMP_DIR}/main"

for test_name in "fibonacci" "sum" "average" "print-star"; do
  print_header "${test_name}"
  ${ARX} --show-llvm-ir --input "examples/${test_name}.arx"
done
