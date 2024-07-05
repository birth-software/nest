#!/bin/bash

function compile()
{
    build_dir=$1
    exe_name=$2
    debug_info=$3
    optimizations=$4

    mkdir -p $build_dir

    compile_command="time clang++ -o $build_dir/$exe_name $debug_info $optimizations -std=gnu++20 -Wall -Wextra -Wpedantic -Wno-nested-anon-types -pedantic -fno-exceptions -fno-stack-protector -ferror-limit=1 -MJ $build_dir/compile_commands.json"

    case "$OSTYPE" in
        darwin*)  ;;
        linux*)   compile_command="$compile_command -ffreestanding -nostdlib -static bootstrap/entry.S" ;;
        *)        echo "unknown: $OSTYPE" ;;
    esac

    compile_command="$compile_command bootstrap/main.cpp"
    echo $compile_command
    eval $compile_command
}
