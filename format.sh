#!/bin/sh

git ls-tree -r HEAD --name-only | grep -E '\.(cc|h)$' | clang-format --verbose -i --files=/dev/stdin
