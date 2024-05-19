#! /usr/bin/env bash


ROOTDIR="$(cd "${0%/*}" 2>/dev/null || exit; pwd)"
cd $ROOTDIR
TEMP="$1.$$"

{
    echo "#include <string>"
    echo "std::string pegof_version = \"$(git describe --tags --abbrev=8 --dirty)\";"
    echo "std::string pcc_version = \"$(git -C packcc describe --tags --abbrev=8 --dirty)\";"
} > "$TEMP"

if [ ! -f "$1" ] || [ "$(md5sum < "$TEMP")" != "$(md5sum < "$1")" ]; then
    mv "$TEMP" "$1"
else
    rm -f "$TEMP"
fi
