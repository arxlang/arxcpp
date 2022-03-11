#!/usr/bin/env bash

if [[ ${CLEAN} == 1 ]]; then
  rm -rf build/*
  rm -f bin/*
fi
