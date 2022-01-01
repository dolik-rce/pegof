#!/usr/bin/env bash

generate_test() {
    if [ -f "$1" ]; then
        echo
        echo "@test \"$(basename "$INPUT") - $2\" {"
        echo "    run_test \"$3\" \"$INPUT\" \"$1\""
        echo "}"
    fi
}

generate_bats() {
    cat <<EOF
#!/usr/bin/env bats
load $TESTDIR/utils.sh
EOF
    for INPUT in "$1"/*.peg; do
        [[ $INPUT =~ _(formatted|optimized).peg ]] && continue
        generate_test "${INPUT/.peg/_formatted.peg}" Formatting --format
        generate_test "${INPUT/.peg/.ast}" AST --ast
        generate_test "${INPUT/.peg/_optimized.ast}" "Optimized AST" "--optimize --ast"
        generate_test "${INPUT/.peg/_optimized.peg}" Optimization --optimize
    done
}


clean() {
    rm -f *.d/test.bats *.d/*.out *.d/*.tmp
}

main() {
    set -e

    if ! which bats &> /dev/null; then
        echo "Warning: 'bats' executable not found on PATH, skipping tests"
        exit 0
    fi

    export ROOTDIR="$(cd "$(dirname "$0")" && pwd)"
    export TESTDIR="$ROOTDIR/tests"
    export PEGOF="$ROOTDIR/build/pegof"
    cd "$TESTDIR"

    clean

    for DIR in *.d; do
        # Do not generate test file if the directory already contains some
        ls "$DIR"/*.bats &> /dev/null && continue
        generate_bats "$DIR" > "$DIR/test.bats"
    done

    echo "Running tests:"
    bats "$@" ./*.d
}

main "$@"
