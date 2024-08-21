#!/usr/bin/env bats

load "$TESTDIR/utils.sh"

check_uncrustify_version() {
    version="$(uncrustify --version)"
    major="$(echo "$version" | cut -d. -f1 | grep -oE '[0-9]+$')"
    minor="$(echo "$version" | cut -d. -f2)"
    [ "$major" -gt 0 ] || { [ "$major" -eq 0 ] && [ "$minor" -ge 72 ]; }
}

test_code_style() {
    if ! command -v "uncrustify" &> /dev/null; then
        skip "uncrustify is not installed"
    elif ! check_uncrustify_version &> /dev/null; then
        skip "uncrustify is too old (minimal required version is 0.72.0)"
    else
        run uncrustify -q -c "$ROOTDIR/uncrustify.cfg" -f "$1"
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
