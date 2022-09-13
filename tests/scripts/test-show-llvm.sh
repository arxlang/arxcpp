#!/usr/bin/env bash

set -e

TEST_DIR_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd . && pwd )"

# load utils functions
. "${TEST_DIR_PATH}/utils.sh"


print_header "fibonnaci"
./build/arx --show-llvm < examples/test_fibonacci.arx
./build/arx --show-llvm --input examples/test_fibonacci.arx

print_header "sum"
./build/arx  --show-llvm  < examples/test_sum.arx
./build/arx  --show-llvm  --input examples/test_sum.arx
