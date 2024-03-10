#!/bin/bash

#set -e
#set -x

rootpath="$(realpath $(dirname "$0"))"
buildpath="$rootpath/build"
mainpath="$rootpath/src/main.cpp"

library_flags="-lpthread -lSDL2 -lGLEW -lGL -lX11 -lm -ldl"
include_flags="-I$rootpath/vendor"
warning_flags="-Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers"
optimization_flags="-ffast-math -fassociative-math -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables"
define_flags="-DTINYOBJ_LOADER_C_IMPLEMENTATION -DSTB_IMAGE_IMPLEMENTATION"
compiler_flags="-std=c++2b $library_flags $include_flags $warning_flags $define_flags $optimization_flags"
compiler_debug_flags="$compiler_flags -O0 -ggdb3 -march=native -DDEBUG -DSLOW -o game_debug"

mkdir "$buildpath" -p
pushd "$buildpath" > /dev/null

echo clang $compiler_debug_flags $mainpath
clang $compiler_debug_flags $mainpath

popd > /dev/null

if command -v jq &>/dev/null 2>&1; then
    read -r -d '' JQ_COMMAND <<'EOF'
    {
        directory: $directory,
        file: $file,
        arguments: $arguments | split(" ") | flatten | select(length > 0)
    }
        | .arguments = [$compiler, $file] + .arguments
        | [.]
EOF
    jq -n \
        --arg directory "$buildpath" \
        --arg file "$mainpath" \
        --arg compiler "clang" \
        --arg arguments "$compiler_debug_flags" \
        "$JQ_COMMAND" | tee 'compile_commands.json'
fi
