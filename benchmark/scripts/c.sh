#!/bin/bash

case "$1" in
setup)
    ${CC:-cc} ${CFLAGS:--O2} "$2.c" -o "$2.tmp"
    sed '/__attribute__/d; s/" FMT_LU "/%lu/g' "packcc/src/packcc.c" > "$2.input"
    ;;
benchmark)
    for i in {1..10}; do
        "$2.tmp" < "$2.input" > /dev/null
    done
    ;;
teardown)
    rm -f "$2.tmp" "$2.input"
    ;;
esac
