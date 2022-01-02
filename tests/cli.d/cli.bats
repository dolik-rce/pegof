#!/usr/bin/env bats
load /home/h/prog/pegof/tests/utils.sh

create_peg() {
    local ID="$BATS_TEST_NUMBER$1"
    PEGFILE="cli.d/$ID.peg.tmp"
    echo "X_$ID<-'X_' '$ID'" > "$PEGFILE"
    echo "X_$ID <- \"X_\" \"$ID\"" > "cli.d/$ID.peg.formatted.tmp"
    echo "X_$ID <- \"X_$ID\"" > "cli.d/$ID.peg.optimized.tmp"
}

check_output() {
    local ID="$BATS_TEST_NUMBER$1"
    [ "$status" -eq 0 ]
    diff --strip-trailing-cr -uN "cli.d/$ID.peg.formatted.tmp" --label "expected" <(echo "$output") --label "actual"
}

check_output_by_string() {
    [ "$status" -eq 0 ]
    diff --strip-trailing-cr -uN <(echo "$1") --label "expected" <(echo "$output") --label "actual"
}

check_output_by_files() {
    check_output_by_string "$(cat "$@")"
}

check_file() {
    local ID="$BATS_TEST_NUMBER$2"
    [ "$status" -eq 0 ]
    diff --strip-trailing-cr -uN "cli.d/$ID.peg.formatted.tmp" --label "expected" "${1:-$PEGFILE}" --label "actual"
}

check_error() {
    [ "$status" -eq 1 ]
    [[ "$output" =~ ${1:-ERROR:} ]]
}

@test "CLI - reading from stdin, writing to stdout" {
    create_peg
    run "$PEGOF" < "$PEGFILE"
    check_output
}

@test "CLI - write to file" {
    create_peg
    run "$PEGOF" --output "$PEGFILE.out" "$PEGFILE"
    check_file "$PEGFILE.out"
}

@test "CLI - inplace formatting" {
    create_peg
    run "$PEGOF" -I "$PEGFILE"
    check_file
}

@test "CLI - optimize" {
    create_peg
    run "$PEGOF" --optimize "$PEGFILE"
    check_output_by_files "${PEGFILE/.tmp/.optimized.tmp}"
}

@test "CLI - multiple files" {
    create_peg A
    A="$PEGFILE"
    create_peg B
    B="$PEGFILE"
    echo "$PEGOF" "$A" "$B" >/dev/stderr
    run "$PEGOF" "$A" "$B"
    check_output_by_files "${A/.tmp/.formatted.tmp}" "${B/.tmp/.formatted.tmp}"
}

@test "CLI - multiple files inplace" {
    create_peg A
    A="$PEGFILE"
    create_peg B
    B="$PEGFILE"
    run "$PEGOF" -I "$A" "$B"
    check_file "$A" A
    check_file "$B" B
}

@test "CLI - multiple files with outputs" {
    create_peg A
    A="$PEGFILE"
    create_peg B
    B="$PEGFILE"
    run "$PEGOF" -o a.out -o b.out -- "$A" "$B"
    check_file "a.out" A
    check_file "b.out" B
}

@test "CLI - multiple files output count mismatch" {
    create_peg A
    A="$PEGFILE"
    create_peg B
    B="$PEGFILE"
    run "$PEGOF" -o a.out "$A" "$B"
    check_error
}

@test "CLI - inplace formatting with --output fails" {
    create_peg
    run "$PEGOF" -I -o - "$PEGFILE"
    check_error
}

@test "CLI - inplace formatting with --ast fails" {
    create_peg
    run "$PEGOF" -I --ast "$PEGFILE"
    check_error
}

@test "CLI - inplace formatting with --debug fails" {
    create_peg
    run "$PEGOF" -I --debug "$PEGFILE"
    check_error
}
