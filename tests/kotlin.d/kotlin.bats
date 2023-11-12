#!/usr/bin/env bats
load /home/h/prog/pegof/tests/utils.sh

@test "kotlin.d - format" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_test "kotlin.d/format.conf" "kotlin.d/kotlin.peg"
    check_status kotlin.d/format.status
    check_stdout "kotlin.d/format.out"
}

@test "kotlin.d - kotlin" {
    if [ -z "$INCLUDE_SLOW_TESTS" ]; then
        skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    fi
    run_test "kotlin.d/kotlin.conf" "kotlin.d/kotlin.peg"
    check_status kotlin.d/kotlin.status
    check_stdout "kotlin.d/kotlin.out"
}
