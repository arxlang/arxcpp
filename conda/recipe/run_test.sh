#!/usr/bin/env bash

TMP_DIR=/tmp/arx-tmp
mkdir -p ${TMP_DIR}

touch "${TMP_DIR}/fibonacci.arx"
cat > "${TMP_DIR}/fibonacci.arx" <<- ARX
function fib(x):
  if x < 3:
    1
  else:
    fib(x-1)+fib(x-2);
ARX

arx --show-ast --input /tmp/src.arx
arx --input "${TMP_DIR}/fibonacci.arx" --output "${TMP_DIR}/fibonacci.o"
