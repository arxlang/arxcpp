#!/usr/bin/env bash

TEST_DIR_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd . && pwd )"

# load utils functions
. "${TEST_DIR_PATH}/utils.sh"

print_header "fibonnaci"
./build/arx --output fibonacci --input examples/test_fibonacci.arx

print_header "sum"
./build/arx --output sum --input examples/test_sum.arx

# teardown
rm -f fibonacci
rm -f sum
