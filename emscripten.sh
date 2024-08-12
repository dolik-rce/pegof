#!/usr/bin/env bash

set -ex

BUILDDIR="${1:-build_js}"
SITEDIR="${2:-_site}"

if [ ! -d "$BUILDDIR/emsdk" ]; then
    mkdir -p "$BUILDDIR"
    git clone https://github.com/emscripten-core/emsdk.git "$BUILDDIR/emsdk"
    "$BUILDDIR"/emsdk/emsdk install latest
    "$BUILDDIR"/emsdk/emsdk activate latest
fi
. "$BUILDDIR"/emsdk/emsdk_env.sh

emcmake cmake -B "$BUILDDIR" -DCMAKE_BUILD_TYPE=Release
export EMCC_CFLAGS="-sEXPORTED_RUNTIME_METHODS=callMain,FS -sMODULARIZE=1 -Wno-unused-command-line-argument"
cmake --build "$BUILDDIR" -v

mkdir -p "$SITEDIR"
cp -v "$BUILDDIR"/pegof.{js,wasm} "$SITEDIR/"
cp -v benchmark/grammars/*.peg "$SITEDIR/"
cp -v web/* "$SITEDIR/"
