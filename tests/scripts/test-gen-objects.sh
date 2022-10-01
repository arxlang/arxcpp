#!/usr/bin/env bash

set -e

DEBUG=""
TMP_DIR=/tmp/arx
mkdir -p "${TMP_DIR}"

if [[ "${1}" == "--debug" ]]; then
  DEBUG="gdb --args"
fi

TEST_DIR_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd . && pwd )"

# load utils functions
. "${TEST_DIR_PATH}/utils.sh"

ARX_CMD="${DEBUG} ./build/arx"

print_header "fibonnaci"
${ARX_CMD} --output "${TMP_DIR}/fibonacci" --input examples/test_fibonacci.arx
# ${ARX_CMD} --output "${TMP_DIR}/fibonacci"  examples/test_fibonacci.arx

print_header "sum"
${ARX_CMD} --output "${TMP_DIR}/sum" --input examples/test_sum.arx
# ${ARX_CMD} --output "${TMP_DIR}/sum" < examples/test_sum.arx
