run_test () {
    "$PEGOF" $1 "$2" > "$3.out"
    diff --strip-trailing-cr -uN "$3" "$3.out"
}
