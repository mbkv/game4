#!/bin/bash

#set -e
#set -x

rootpath="$(realpath $(dirname "$0"))"
buildpath="$rootpath/build"
mainpath="$rootpath/src/main.cpp"

library_flags="-lpthread -lSDL2 -lGLEW -lGL -lX11 -lm -ldl"
include_flags="-I$rootpath"
warning_flags="-Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers"
optimization_flags="-msse4.2 -ffast-math -fassociative-math -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables"
define_flags="-DTINYOBJ_LOADER_C_IMPLEMENTATION -DSTB_IMAGE_IMPLEMENTATION -DFAST_OBJ_IMPLEMENTATION"
compiler_flags="-std=c++2b $library_flags $include_flags $warning_flags $define_flags $optimization_flags"
compiler_debug_flags="$compiler_flags -ggdb3 -O0 -march=native -DDEBUG -DSLOW"

mkdir "$buildpath" -p
pushd "$buildpath" > /dev/null

echo clang++ $compiler_debug_flags $mainpath -o game_debug
clang++ $compiler_debug_flags $mainpath -o game_debug

# echo emcc --use-port=sdl2 -s USE_WEBGL2=1 $compiler_debug_flags $mainpath -o game_debug.html
# emcc --use-port=sdl2 -s USE_WEBGL2=1 -mrelaxed-simd $compiler_debug_flags $mainpath -o game_debug.html


popd > /dev/null

if command -v jq &>/dev/null 2>&1; then
    read -r -d '' JQ_COMMAND <<'EOF'
    {
        directory: $directory,
        file: $file,
        arguments: $arguments | split(" ") | flatten | select(length > 0)
    } | [.]
EOF
    jq -n \
        --arg directory "$buildpath" \
        --arg file "$mainpath" \
        --arg arguments "emcc $compiler_debug_flags $mainpath" \
        "$JQ_COMMAND" | tee 'compile_commands.json'
fi
