#!/bin/bash

main() {
    set -e -o pipefail

    export ROOTDIR="$(cd "$(dirname "$0")/.." && pwd)"
    export PEGOF="$1"
    export OPTS="${OPTS:---optimize all}"

    cd "$ROOTDIR"

    for GRAMMAR in benchmark/grammars/*.peg; do
        BASE="$(basename "$GRAMMAR" .peg)"
        echo
        echo "# $BASE"
        "$PEGOF" $OPTS --benchmark "benchmark/scripts/$BASE.sh" --output "/dev/null" -i "$GRAMMAR"
    done
}

main "$@"
