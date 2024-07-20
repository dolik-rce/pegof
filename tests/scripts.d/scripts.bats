#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

normalize_output() {
    sed '
        s/pegof_[0-9]\+/pegof_<random>/g;
        s/| [ 0-9]\{9\}%/|  <number>%/g;
        s/| [ 0-9]\{10\}/|   <number>/g;
    ' <<<"$output"
}

@test "scripts.d - benchmark" {
    run_test "scripts.d/benchmark.conf" "scripts.d/scripts.peg"
    check_status scripts.d/benchmark.status
    output="$(normalize_output)"
    check_stdout "scripts.d/benchmark.out"
    check_file "scripts.d/benchmark.expected" "scripts.d/benchmark.tmp"
}

@test "scripts.d - debug" {
    run_test "scripts.d/debug.conf" "scripts.d/scripts.peg"
    check_status scripts.d/debug.status
    output="$(normalize_output)"
    check_stdout "scripts.d/debug.out"
}
