#!/usr/bin/env bash

set -x

PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

IWYU_PATCH_DIR=/tmp/iwyu
mkdir -p ${IWYU_PATCH_DIR}

IWYU_PATCH_FILEPATH=${IWYU_PATCH_DIR}/iwyu.patch
echo "" > ${IWYU_PATCH_FILEPATH}

include-what-you-use \
  -Xiwyu \
  --verbose=0 \
  -I${CONDA_PREFIX}/include \
  -I${PROJECT_PATH}/arx/include \
  -I${CONDA_PREFIX}/x86_64-conda-linux-gnu/include/c++/10.3.0 \
  -I${CONDA_PREFIX}/x86_64-conda-linux-gnu/include/c++/10.3.0/x86_64-conda-linux-gnu \
  -std=c++17 \
  ${1} 2> ${IWYU_PATCH_FILEPATH}

if [ -s ${IWYU_PATCH_FILEPATH} ]
then
     # cat ${IWYU_PATCH_FILEPATH} | grep -v error > ${IWYU_PATCH_FILEPATH}
     fix_includes.py < ${IWYU_PATCH_FILEPATH}
     # rm ${IWYU_PATCH_FILEPATH}
     exit 1
fi
