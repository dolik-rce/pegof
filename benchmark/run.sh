#!/bin/bash

main() {
    set -e -o pipefail

    export ROOTDIR="$(cd "$(dirname "$0")/.." && pwd)"
    export PEGOF="build/pegof"
    export PACKCC="${PACKCC:-packcc}"
    export OPTS="${OPTS:---optimize all}"

    cd "$ROOTDIR"
    make -C build pegof

    for GRAMMAR in benchmark/grammars/*.peg; do
        BASE="$(basename "$GRAMMAR" .peg)"
        echo
        echo "# $BASE"
        "$PEGOF" $OPTS --benchmark "benchmark/scripts/$BASE.sh" --output "/dev/null" -i "$GRAMMAR"
    done
}

main "$@"
