#!/bin/sh

CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
git ls-tree -r HEAD --name-only | grep -E '\.(cc|h)$' | "$CLANG_FORMAT" --verbose -i --files=/dev/stdin
