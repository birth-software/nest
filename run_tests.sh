#!/bin/bash

set -e
source ./compile.sh
build_dir="build"
exe_name="nest"
exe_path=$build_dir/$exe_name
debug_flags="-g"
optimization_flags=""
test_names="first"

compile $build_dir $exe_name $debug_flags $optimization_flags;

printf "\n======================\n"
printf "TESTS"
printf "\n======================\n\n"
for test_name in "${test_names[@]}"
do
    echo "$test_name..."
    build/nest "tests/$test_name.nat"
    echo "$test_name [COMPILATION] [OK]"
    nest/$test_name
    echo "$test_name [RUN] [OK]"
done

