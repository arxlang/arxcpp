#!/usr/bin/env bash

touch /tmp/src.arx
cat > /tmp/src.arx <<- ARX
function fib(x):
  if x < 3:
    1
  else:
    fib(x-1)+fib(x-2);

fib(10)

ARX
arx --show-llvm < /tmp/src.arx
