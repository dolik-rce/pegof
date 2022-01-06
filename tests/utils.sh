run_test () {
    echo "# RUNNING: $PEGOF $1 $2 > $3.out" > /dev/stderr
    "$PEGOF" $1 "$2" > "$3.out"
    diff --strip-trailing-cr -uN "$3" "$3.out"
}
