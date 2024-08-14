#!/usr/bin/env bats
load "$TESTDIR/utils.sh"

REPO="$PWD/ctags.d/ctags.git"
CPU="$(nproc || echo 1)"

@test "ctags.d - setup" {
    [ "$INCLUDE_SLOW_TESTS" ] || skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    if [ -d "$REPO" ]; then
        git -C "$REPO" pull
    else
        git clone https://github.com/universal-ctags/ctags.git "$REPO"
        (cd "$REPO" && ./autogen.sh && ./configure --with-pegof="$PEGOF")
    fi
}

@test "ctags.d - process grammars" {
    [ "$INCLUDE_SLOW_TESTS" ] || skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    [ -e "$REPO/Makefile" ] || skip "This test requires 'ctags.d - setup' to run first"
    run make -C "$REPO" peg/*.pego -j$CPU
}

@test "ctags.d - compile" {
    [ "$INCLUDE_SLOW_TESTS" ] || skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    [ -e "$REPO/Makefile" ] || skip "This test requires 'ctags.d - setup' to run first"
    run make -C "$REPO" -j$CPU
}

@test "ctags.d - run tests" {
    [ "$INCLUDE_SLOW_TESTS" ] || skip "slow test (set INCLUDE_SLOW_TESTS=1 to run this)"
    [ -e "$REPO/ctags" ] || skip "This test requires 'ctags.d - compile' to run first"
    run make -C "$REPO" units LANGUAGES="$($REPO/ctags --list-languages=_packcc | tr '\n' ',')"
}
