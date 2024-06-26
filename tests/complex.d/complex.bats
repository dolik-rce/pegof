#!/usr/bin/env bats
load "$TESTDIR/utils.sh"

PEGOF="${PEGOF/_test/}"

@test "complex.d - kotlin format" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_test "complex.d/kotlin_format.conf" "complex.d/kotlin.peg"
    check_status complex.d/kotlin_format.status
    check_stdout "complex.d/kotlin_format.out"
}

@test "complex.d - kotlin optimization" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_test "complex.d/kotlin.conf" "complex.d/kotlin.peg"
    check_status complex.d/kotlin.status
    check_stdout "complex.d/kotlin.out"
}

@test "complex.d - C format" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_test "complex.d/c_format.conf" "complex.d/c.peg"
    check_status complex.d/c_format.status
    check_stdout "complex.d/c_format.out"
}

@test "complex.d - C optimization" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_test "complex.d/c.conf" "complex.d/c.peg"
    check_status complex.d/c.status
    check_stdout "complex.d/c.out"
}

@test "complex.d - JSON format" {
    run_test "complex.d/json_format.conf" "complex.d/json.peg"
    check_status complex.d/json_format.status
    check_stdout "complex.d/json_format.out"
}

@test "complex.d - JSON optimization" {
    run_test "complex.d/json.conf" "complex.d/json.peg"
    check_status complex.d/json.status
    check_stdout "complex.d/json.out"
}

@test "complex.d - calc format" {
    run_test "complex.d/calc_format.conf" "complex.d/calc.peg"
    check_status complex.d/calc_format.status
    check_stdout "complex.d/calc_format.out"
}

@test "complex.d - calc optimization" {
    run_test "complex.d/calc.conf" "complex.d/calc.peg"
    check_status complex.d/calc.status
    check_stdout "complex.d/calc.out"
}
