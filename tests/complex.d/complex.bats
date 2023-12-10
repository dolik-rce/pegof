#!/usr/bin/env bats
load /home/h/prog/pegof/tests/utils.sh

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
