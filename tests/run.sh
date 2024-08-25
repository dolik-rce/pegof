#!/usr/bin/env bash

get_inputs() {
    sed -n '/^input/s/input //p;' "$1"
}

get_outputs() {
    if grep -q inplace "$1"; then
        get_inputs "$1"
    else
        sed -n '/^output/s/output //p;' "$1"
    fi
}

generate_bats() {
    cat <<EOF
#!/usr/bin/env bats
load "$TESTDIR/utils.sh"
EOF
    if [ -e "$1/setup.sh" ]; then
        echo "load \"$TESTDIR/$1/setup.sh\""
    fi
    for CONF in "$1"/*.conf; do
        [ -e "$CONF" ] || continue
        BASE="$(basename "$CONF" .conf)"
        mapfile -t INPUTS <<<"$(get_inputs "$CONF")"
        mapfile -t OUTPUTS <<<"$(get_outputs "$CONF")"
        echo
        echo "@test \"$(dirname "$CONF") - $BASE\" {"
        echo "    # inputs: ${INPUTS[*]}"
        echo "    # outputs: ${OUTPUTS[*]}"
        INPUT=""
        [ -e "${CONF//.conf/.in}" ] && INPUT=" < ${CONF//.conf/.in}"
        echo "    run_test \"$CONF\" \"$(get_inputs "$CONF")\"$INPUT"
        echo "    check_status ${CONF//.conf/}.status"
        [ -e "${CONF//.conf/.out}" ] && echo "    check_stdout \"${CONF//.conf/.out}\""
        for OUTPUT in "${OUTPUTS[@]}"; do
            if [ -e "$OUTPUT.$BASE.expected" ]; then
                echo "    check_file \"$OUTPUT.$BASE.expected\" \"$OUTPUT.tmp\""
            elif [ -e "${OUTPUT//.tmp/.expected}" ]; then
                echo "    check_file \"${OUTPUT//.tmp/.expected}\" \"$OUTPUT\""
            fi
        done
        echo "    clean_up \"$(IFS=$'\n'; echo "${INPUTS[*]}")\""
        echo "}"
    done
}

clean() {
    rm -f "$TESTDIR"/*.d/generated.bats "$TESTDIR"/*.d/*.tmp "$TESTDIR"/**/*.processed.{c,h} "$BUILDDIR"/**/*.gcda
}

main() {
    set -e
    shopt -s globstar

    if ! which bats &> /dev/null; then
        echo "Warning: 'bats' executable not found on PATH, skipping tests"
        exit 0
    fi

    export TESTDIR="$(cd "$(dirname "$0")" && pwd)"
    export ROOTDIR="$TESTDIR/.."
    export BUILDDIR="${BUILDDIR:-$ROOTDIR/build}"
    export PEGOF="${PEGOF:-$BUILDDIR/pegof}"
    cd "$TESTDIR"

    echo "Cleaning files from previous runs..."
    clean

    echo "Generating tests..."
    for DIR in *.d; do
        # Do not generate test file if the directory already contains some
        ls "$DIR"/*.bats &> /dev/null && continue
        generate_bats "$DIR" > "$DIR/generated.bats"
    done

    echo "Running tests:"
    bats "$@" "$PWD" "$PWD"/*.d
}

main "$@"
