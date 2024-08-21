#!/usr/bin/env bash

OPTS=()
ARGS=(-p)
while [ $# -gt 0 ]; do
    case "$1" in
        --ascii|--lines|--debug|-a|-l|-d)
            OPTS+=("${1//-/}") ;;
        *)
            ARGS+=("$1") ;;
    esac
    shift
done

if [ "$OPTS" ]; then
    ARGS+=(-P "$(IFS=,; echo "${OPTS[@]}")")
fi
if [ "$OPTIMIZE" = "1" ]; then
    ARGS+=(-O all)
fi

echo "RUNNING: $PEGOF ${ARGS[*]}" >> /dev/stderr
"$PEGOF" "${ARGS[@]}"
