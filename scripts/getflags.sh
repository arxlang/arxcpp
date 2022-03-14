#!/usr/bin/env bash

llvm-config --cxxflags --ldflags --system-libs --libs core | sed 's/-fno-exceptions//g'
