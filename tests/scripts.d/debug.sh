#!/usr/bin/env bash

set -e
echo "Debug script called: $*"
# Let's fail when rule C is inlined
if ! grep -q "C" "$1"; then
    echo "Detected problem in $1:"
    cat "$1"
    exit 42
fi
