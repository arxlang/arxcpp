#!/usr/bin/env bash

PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

cd ${PROJECT_PATH}/build
ninja --verbose tidy
