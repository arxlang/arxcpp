#!/usr/bin/env bash

# note: sync --libs with CMakeLists.txt
llvm-config \
  --cxxflags \
  | sed 's/-fno-exceptions//g' \
  | sed 's/-lLLVM-13//g'
