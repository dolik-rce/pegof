#! /usr/bin/env bash


ROOTDIR="$(cd "${0%/*}" 2>/dev/null || exit; pwd)"
cd $ROOTDIR

{
    sed -n '1,/^## Usage:/ p' README.md
    build/pegof --usage-markdown 2>&1
    sed -n '/## Configuration file/,$ p' README.md

} > _README.md

mv _README.md README.md
