#!/usr/bin/env bash

set -e

if ! which gcovr &> /dev/null; then
    echo "Error: gcovr is not installed!"
    exit 1
fi

cd "$(dirname "$0")/.."
tests/run.sh

mkdir -p "tests/coverage"
gcovr -s --calls -e '.*/packcc.c' -r "$PWD" . --html-details="tests/coverage/report.html"
echo "Coverage report: file://$PWD/tests/coverage/report.html"
