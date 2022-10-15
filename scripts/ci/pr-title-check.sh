#!/usr/bin/env bash

set -e

PREFIXES="build|ci|docs|perf|refactor|test|chore|feat|fix|BREAKING CHANGE"

echo "${1}" | grep -E "^(${PREFIXES})(\(.+\))?\:"
