#!/usr/bin/env bash

print_header () {
  python -c "print('#' * 80)"
  echo "# ${1}"
  python -c "print('#' * 80)"
}
