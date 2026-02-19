#!/usr/bin/env bats
load "$TESTDIR/utils.sh"

run_pegof_test() {
    run_test "complex.d/${1}_${2}.conf" "complex.d/$1.peg"
    check_status "complex.d/${1}_${2}.status"
    check_stdout "complex.d/${1}_${2}.out"
    cat <<<"$output" > "complex.d/${1}_${2}.peg.tmp"
}

run_compile_test() {
    [ -f "complex.d/${1}_${2}.peg.tmp" ] || skip "This test requires 'complex.d - $3' to run first"
    "$PEGOF" -o "complex.d/${1}_${2}.processed" -p "complex.d/${1}_${2}.peg.tmp"
    ${CC:-cc} ${CFLAGS:--O2} "complex.d/${1}_${2}.processed.c" -o "complex.d/${1}_${2}.tmp"
}

run_parser_test() {
    [ -f "complex.d/${1}_${2}.tmp" ] || skip "This test requires 'complex.d - $3' to run first"
    run "complex.d/${1}_${2}.tmp" < "complex.d/${1}.in"
    check_stdout "complex.d/${1}_run.out"
}

@test "complex.d - Kotlin format" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_pegof_test "kotlin" "format"
}

@test "complex.d - Kotlin compile formatted" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_compile_test "kotlin" "format" "Kotlin format"
}

@test "complex.d - Kotlin run formatted" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_parser_test "kotlin" "format" "Kotlin compile formatted"
}

@test "complex.d - Kotlin optimize" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_pegof_test "kotlin" "optimize"
}

@test "complex.d - Kotlin compile optimized" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_compile_test "kotlin" "optimize" "Kotlin optimized"
}

@test "complex.d - Kotlin run optimized" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_parser_test "kotlin" "optimize" "Kotlin compile optimized"
}

@test "complex.d - C format" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_pegof_test "c" "format"
}

@test "complex.d - C compile formatted" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_compile_test "c" "format" "C format"
}

@test "complex.d - C run formatted" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_parser_test "c" "format" "C compile formatted"
}

@test "complex.d - C optimize" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_pegof_test "c" "optimize"
}

@test "complex.d - C compile optimized" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_compile_test "c" "optimize" "C optimized"
}

@test "complex.d - C run optimized" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_parser_test "c" "optimize" "C compile optimized"
}

@test "complex.d - JSON format" {
    run_pegof_test "json" "format"
}

@test "complex.d - JSON compile formatted" {
    run_compile_test "json" "format" "JSON format"
}

@test "complex.d - JSON run formatted" {
    run_parser_test "json" "format" "JSON compile formatted"
}

@test "complex.d - JSON optimize" {
    run_pegof_test "json" "optimize"
}

@test "complex.d - JSON compile optimized" {
    run_compile_test "json" "optimize" "JSON optimized"
}

@test "complex.d - JSON run optimized" {
    run_parser_test "json" "optimize" "JSON compile optimized"
}

@test "complex.d - calc format" {
    run_pegof_test "calc" "format"
}

@test "complex.d - calc compile formatted" {
    run_compile_test "calc" "format" "calc format"
}

@test "complex.d - calc run formatted" {
    run_parser_test "calc" "format" "calc compile formatted"
}

@test "complex.d - calc optimize" {
    run_pegof_test "calc" "optimize"
}

@test "complex.d - calc compile optimized" {
    run_compile_test "calc" "optimize" "calc optimize"
}

@test "complex.d - calc run optimized" {
    run_parser_test "calc" "optimize" "calc compile optimized"
}
