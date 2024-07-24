#!/bin/bash

function compile()
{
    mkdir -p $build_dir

    build_dir=$1
    exe_name=$2
    debug_info=$3
    optimizations=$4
    static=$5

    echo "BUILD_DIR: $build_dir"
    echo "EXE_NAME: $exe_name"
    echo "DEBUG_INFO: $debug_info"
    echo "OPTIMIZATIONS: $optimizations"
    echo "STATIC: $static"

    compile_command="clang bootstrap/main.c -o $build_dir/$exe_name $debug_info $optimizations -std=gnu2x -Wall -Wextra -Wpedantic -Wno-nested-anon-types -Wno-keyword-macro -Wno-gnu-auto-type -Wno-auto-decl-extensions -pedantic -fno-exceptions -fno-stack-protector -ferror-limit=1 -MJ $build_dir/compile_commands.json"

    if [ "$static" == "1" ]
    then
        compile_command="$compile_command -ffreestanding -nostdlib -static -DSTATIC"
    fi

    echo -e "\x1b[36m$compile_command\x1b[0m"
    eval "time $compile_command"
}
