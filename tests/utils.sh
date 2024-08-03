run_test() {
    echo "CMD: $PEGOF -c $1" > /dev/stderr
    mapfile -t INPUTS <<<"$2"
    for INPUT in "${INPUTS[@]}"; do
        [ "$INPUT" ] || continue
        cp "$INPUT" "$INPUT".backup
    done
    run "$PEGOF" -c "$1"
    for INPUT in "${INPUTS[@]}"; do
        [ "$INPUT" ] || continue
        if ! cmp -s "$INPUT.backup" "$INPUT"; then
            cp "$INPUT" "$INPUT.tmp"
            mv -f "$INPUT.backup" "$INPUT"
        else
            rm -f "$INPUT.backup"
        fi
    done
}

clean_up() {
    mapfile -t INPUTS <<<"$1"
    for INPUT in "${INPUTS[@]}"; do
        [ -e "$INPUT.tmp" ] || continue
        rm -f "$INPUT.tmp"
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
    if [ ! -e "$1" ]; then
        echo "File '$1' does not exist!"
        return 1
    elif [ "$(cat "$1")" == "$output" ]; then
        return 0
    fi
    diff --strip-trailing-cr -u "$1" --label "$1" <(echo "$output") --label stdout
}

check_file() {
    diff --strip-trailing-cr -u "$1" "$2"
}
