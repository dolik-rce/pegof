run_test() {
    echo "CMD: $PEGOF -c $CONF" > /dev/stderr
    mapfile -t INPUTS <<<"$2"
    for INPUT in "${INPUTS[@]}"; do
        [ "$INPUT" ] || continue
        cp "$INPUT" "$INPUT".backup
    done
    run "$PEGOF" -c "$1"
    for INPUT in "${INPUTS[@]}"; do
        [ "$INPUT" ] || continue
        mv -f "$INPUT".backup "$INPUT"
    done
}

check_status() {
    if [ -e "$1" ]; then
        EXP="$(cat "$1")"
    else
        EXP=0
    fi
    if [ "$status" -ne "$EXP" ]; then
        echo "Expected status code '$EXP', but pegof returned '$status'!" > /dev/stderr
        return 1
    fi
}

check_stdout() {
    diff --strip-trailing-cr -uN <(echo "$output") --label stdout "$1"
}

check_file() {
    diff --strip-trailing-cr -uN "$1" "$2"
}

check_packcc_compatibility() {
    if [ $# -gt 0 ]; then
        "$PACKCC" -o "$1.processed" "$1"
    else
        echo "$output"
        "$PACKCC" -o "stdout.processed" <<<"$output"
    fi
}
