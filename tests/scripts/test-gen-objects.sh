#!/usr/bin/env bash

set -e

TMP_DIR=/tmp/arx
mkdir -p "${TMP_DIR}"

TEST_DIR_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd . && pwd )"

# load utils functions
. "${TEST_DIR_PATH}/utils.sh"

print_header "fibonnaci"
./build/arx --output ${TMP_DIR}/fibonacci < examples/test_fibonacci.arx
./build/arx --output ${TMP_DIR}/fibonacci --input examples/test_fibonacci.arx

print_header "sum"
./build/arx --output ${TMP_DIR}/sum < examples/test_sum.arx
./build/arx --output ${TMP_DIR}/sum --input examples/test_sum.arx
