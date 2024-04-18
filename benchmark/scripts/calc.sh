#!/bin/bash

case "$1" in
setup)
    ${CC:-cc} ${CFLAGS:--O2} "$2.c" -o "$2.tmp"
    ;;
benchmark)
    for i in {1..10}; do
        "$2.tmp" < "benchmark/inputs/calc.txt" > /dev/null
    done
    ;;
teardown)
    rm -f "$2.tmp"
    ;;
esac
