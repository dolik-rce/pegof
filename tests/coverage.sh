#!/usr/bin/env bash

set -e

if ! which lcov &> /dev/null; then
    echo "Error: lcov is not installed!"
    exit 1
fi

cd "$(dirname "$0")/.."
tests/run.sh

COVDIR="$PWD/tests/coverage"
mkdir -p "$COVDIR"
rm -rf "$COVDIR"/*

lcov --capture --rc lcov_branch_coverage=1 --base-directory "$PWD" --directory ./ --no-external --exclude "$PWD/packcc" --output "$COVDIR/lcov.info"
genhtml --flat --show-navigation --header-title "Pegof test coverage" --legend -o tests/coverage "$COVDIR/lcov.info"

echo "Coverage report: file://$COVDIR/index.html"
