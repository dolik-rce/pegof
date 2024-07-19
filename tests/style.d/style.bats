#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_bats_version() {
    version="$(bats --version | cut -d" " -f2)"
    major="$(echo "$version" | cut -d. -f1 | grep -oE '[0-9]+$')"
    minor="$(echo "$version" | cut -d. -f2)"
    [ "$major" -gt 1 ] || { [ "$major" -eq 1 ] && [ "$minor" -ge 11 ]; }
}

check_clang_format_version() {
    major="$(clang-format --version | grep -Eo '[0-9]+.[0-9]+.[0-9]+' | grep -o '^[0-9]*')"
    [ "$major" -ge "$1" ]
}

test_code_style() {
    if ! command -v "clang-format" &> /dev/null; then
        skip "clang-format is not installed"
    elif ! check_clang_format_version 18; then
        skip "requires clang-format version 18 or higher"
    else
        run clang-format "$1"
        [ "$status" -eq 0 ]
        diff --strip-trailing-cr -uN "$1" --label "$1" <(echo "$output") --label "formatted"
    fi
}

if check_bats_version 1 11; then
    # nice looking version, that tests each file separately (requires bats 1.11.0 or newer)
    BTF="bats_test_function"
    for FILE in "$ROOTDIR"/src{,/ast}/*.{h,cc}; do
        $BTF --description "style.d - ${FILE/$ROOTDIR\/src\//}" -- test_code_style "$FILE"
    done
else
    # backwards-compatible version of the test for older bats, tests all files in single test
    @test "style.d - check all source files" {
        for FILE in "$ROOTDIR"/src{,/ast}/*.{h,cc}; do
            test_code_style "$FILE"
        done
    }
fi
