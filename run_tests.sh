#!/bin/bash

set -e
source ./compile.sh
all=$1
build_dir="build"
exe_name="nest"
exe_path=$build_dir/$exe_name
debug_flags="-g"
no_optimization_flags=""
test_names="first"

if [ "$all" == "1" ]
then
    optimization_modes=("" "-O1" "-O2 march=native", "-Os march=native" "-Oz march=native" "-O3 march=native")
else
    optimization_modes=("")
fi

for optimization_mode in "${optimization_modes[@]}"
do

    printf "\n===========================\n"
    echo "TESTS ($optimization_mode)"
    printf "===========================\n\n"

    compile $build_dir $exe_name $debug_flags $optimization_mode;

    printf "\n===========================\n"
    echo "COMPILER BUILD OK"
    printf "===========================\n\n"

    for test_name in "${test_names[@]}"
    do
        printf "\n===========================\n"
        echo "$test_name..."
        printf "===========================\n\n"
        build/nest "tests/$test_name.nat"
        printf "\n===========================\n"
        echo "$test_name [COMPILATION] [OK]"
        printf "===========================\n\n"
        nest/$test_name
        printf "\n===========================\n"
        echo "$test_name [RUN] [OK]"
        printf "===========================\n\n"
    done
done

