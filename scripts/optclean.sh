#!/usr/bin/env bash

if [[ ${CLEAN} == 1 ]]; then
  rm -rf build/*
  rm -f bin/*
  find . -name "*.gcda" -print0 | xargs -0 rm
fi
