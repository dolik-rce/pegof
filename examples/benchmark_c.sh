#!/bin/bash

case "$1" in
setup)
    # Compiling grammar
    ${CC:-cc} ${CFLAGS:--O2} "$2.c" -o "$2.tmp"
    sed '/__attribute__/d; s/" FMT_LU "/%lu/g' "packcc/src/packcc.c" > "input.c"
    ;;
benchmark)
    # Running benchmark
    for i in {1..10}; do
        "$2.tmp" < "input.c" > /dev/null
    done
    ;;
teardown)
    # Cleaning up
    rm -f "$2.tmp" "input.c"
    ;;
esac
