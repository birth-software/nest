#!/bin/bash

function compile()
{
    build_dir=$1
    exe_name=$2
    debug_info=$3
    optimizations=$4

    mkdir -p $build_dir

    compile_command="clang++ -o $build_dir/$exe_name $debug_info $optimizations -std=gnu++20 -Wall -Wextra -Wpedantic -Wno-nested-anon-types -pedantic -fno-exceptions -fno-stack-protector -ferror-limit=1 -MJ $build_dir/compile_commands.json"

    case "$OSTYPE" in
        darwin*)  compile_command="$compile_command -DDEMAND_LIBC=1";;
        linux*)   compile_command="$compile_command -ffreestanding -nostdlib -static bootstrap/entry.S -DDEMAND_LIBC=0" ;;
        *)        echo "Unknown operating system $OSTYPE: no specific flags were added" ;;
    esac

    compile_command="$compile_command bootstrap/main.cpp"
    echo -e "\x1b[36m$compile_command\x1b[0m"
    eval "time $compile_command"
}
