run_test() {
    echo "CMD: $PEGOF -c $1" > /dev/stderr
    INPLACE="$(grep -q inplace "$1" && echo "1" || echo "0")"
    if [ "$INPLACE" = 1 ]; then
        mapfile -t INPUTS <<<"$2"
        for INPUT in "${INPUTS[@]}"; do
            [ "$INPUT" ] || continue
            cp "$INPUT" "$INPUT".backup
        done
    fi
    run "$PEGOF" -c "$1"
    if [ "$INPLACE" = 1 ]; then
        for INPUT in "${INPUTS[@]}"; do
            [ "$INPUT" ] || continue
            if ! cmp -s "$INPUT.backup" "$INPUT"; then
                cp "$INPUT" "$INPUT.tmp"
                mv -f "$INPUT.backup" "$INPUT"
            else
                rm -f "$INPUT.backup"
            fi
        done
    fi
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

check_bats_version() {
    version="$(bats --version | cut -d" " -f2)"
    major="$(echo "$version" | cut -d. -f1 | grep -oE '[0-9]+$')"
    minor="$(echo "$version" | cut -d. -f2)"
    [ "$major" -gt $1 ] || { [ "$major" -eq $1 ] && [ "$minor" -ge $2 ]; }
}
