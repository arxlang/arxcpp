#!/usr/bin/env bash

set -e

echo "${1}" | grep -E "^(BREAKING CHANGE|chore|fix|docs|feat|test)(\(.+\))?\:"
