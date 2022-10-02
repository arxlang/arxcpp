#!/usr/bin/env bash

set -x

PROJECT_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && cd .. && pwd )"

IWYU_PATCH_DIR=/tmp/iwyu
mkdir -p ${IWYU_PATCH_DIR}

IWYU_PATCH_FILEPATH=${IWYU_PATCH_DIR}/iwyu.patch
touch ${IWYU_PATCH_FILEPATH}
EXIT_CODE=0

GCC_INCLUDE_PATH=$(dirname $(find ${CONDA_PREFIX} -name cxxabi.h))
CLANG_INCLUDE_PATH=$(dirname $(find ${CONDA_PREFIX}/lib/clang/ -name builtins.h))

for FILEPATH in "$@"
do
  echo "" > ${IWYU_PATCH_FILEPATH}

  include-what-you-use \
    -Xiwyu \
    --error_always \
    --verbose 0 \
    -I${CONDA_PREFIX}/include \
    -I${PROJECT_PATH}/src \
    -I${GCC_INCLUDE_PATH} \
    -I${GCC_INCLUDE_PATH}/x86_64-conda-linux-gnu \
    -I${CLANG_INCLUDE_PATH} \
    -std=c++20 \
    ${FILEPATH} 2> ${IWYU_PATCH_FILEPATH}

  if [ -s ${IWYU_PATCH_FILEPATH} ]
  then
    # fix_includes.py < ${IWYU_PATCH_FILEPATH}
    grep --count --max-count 1 "The full include-list for" ${IWYU_PATCH_FILEPATH} | grep 0
    NEW_EXIT_CODE=$?
    cat ${IWYU_PATCH_FILEPATH}
    rm -f ${IWYU_PATCH_FILEPATH}
    EXIT_CODE=$(expr $EXIT_CODE + $NEW_EXIT_CODE)
  fi
done

exit ${EXIT_CODE}
