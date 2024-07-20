#!/usr/bin/env bash

set -e

case "$1" in
setup)
    echo "Setup called: $*"
    ;;
benchmark)
    echo "Benchmark called: $*"
    ;;
teardown)
    echo "Teardown called: $*"
    ;;
esac
