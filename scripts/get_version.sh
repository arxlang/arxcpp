#!/usr/bin/env bash

function get_clang_version() {
    clang++ --version | grep -o "[0-9]*\.[0-9]*\.[0-9]*"
}

function get_gcc_version() {
    gcc --version | grep -o "[0-9]*\.[0-9]*\.[0-9]*$"
}

get_clang_version
get_gcc_version
