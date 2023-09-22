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
        if ! cmp -s "$INPUT".backup "$INPUT"; then
            mv -f "$INPUT".backup "$INPUT"
        else
            rm -f "$INPUT".backup
        fi
        done
}

check_status() {
    if [ -e "$1" ]; then
        EXP="$(cat "$1")"
    else
        EXP=0
    fi
    if [ "$status" -ne "$EXP" ]; then
        echo "Expected status code '$EXP', but pegof returned '$status'!"$'\n'"$output" > /dev/stderr
        return 1
    fi
}

check_stdout() {
    diff --strip-trailing-cr -uN <(echo "$output") --label stdout "$1"
}

check_file() {
    diff --strip-trailing-cr -uN "$1" "$2"
}
