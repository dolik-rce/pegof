#!/usr/bin/env bash

get_inputs() {
    sed -n '/^input/s/input\s\+//p;' "$1"
}

get_outputs() {
    if grep -q inplace "$1"; then
        get_inputs "$1"
    else
        sed -n '/^output/s/output\s\+//p;' "$1"
    fi
}

generate_bats() {
    cat <<EOF
#!/usr/bin/env bats
load $TESTDIR/utils.sh
EOF
    for CONF in "$1"/*.conf; do
        [ -e "$CONF" ] || continue
        echo
        echo "@test \"$(dirname "$CONF") - $(basename "$CONF" .conf)\" {"
        INPUT=""
        [ -e "${CONF//.conf/.in}" ] && INPUT=" < ${CONF//.conf/.in}"
        echo "    run_test \"$CONF\" \"$(get_inputs "$CONF")\"$INPUT"
        echo "    check_status ${CONF//.conf/}.status"
        [ -e "${CONF//.conf/.out}" ] && echo "    check_stdout \"${CONF//.conf/.out}\""
        mapfile -t OUTPUTS <<<"$(get_outputs "$CONF")"
        for OUTPUT in "${OUTPUTS[@]}"; do
            [ -e "${OUTPUT//.tmp/.expected}" ] || continue
            echo "    check_file \"$OUTPUT\" \"${OUTPUT//.tmp/.expected}\""
        done
        for OUTPUT in "${OUTPUTS[@]}"; do
            [ -f "${CONF/.conf/.status}" ] && continue
            grep -q -e ast -e debug "$CONF" && continue
            if [ "$OUTPUT" ]; then
                echo "    check_packcc_compatibility \"$OUTPUT\""
            else
                echo "    check_packcc_compatibility"
            fi
        done
        echo "}"
    done
}


clean() {
    rm -f ./*.d/generated.bats ./*.d/*.tmp
}

main() {
    set -e

    if ! which bats &> /dev/null; then
        echo "Warning: 'bats' executable not found on PATH, skipping tests"
        exit 0
    fi

    export TESTDIR="$(cd "$(dirname "$0")" && pwd)"
    export ROOTDIR="$TESTDIR/.."
    export PEGOF="$ROOTDIR/build/pegof"
    export PACKCC="${PACKCC:-packcc}"
    cd "$TESTDIR"

    clean

    for DIR in *.d; do
        # Do not generate test file if the directory already contains some
        ls "$DIR"/*.bats &> /dev/null && continue
        generate_bats "$DIR" > "$DIR/generated.bats"
    done

    echo "Running tests:"
    bats "$@" ./*.d
}

main "$@"
