#!/bin/bash

set -e
source ./compile.sh
all=$1
all=$1
build_dir="build"
base_exe_name="nest"
debug_flags="-g"
no_optimization_flags=""
test_names=(
    "first"
    "add_sub"
    "mul"
    "div"
    "and"
    "or"
    "xor"
    "return_var"
    "return_mod_scope"
    "shift_left"
    "shift_right"
)

if [ "$all" == "1" ]
then
    optimization_modes=("-O0" "-O1" "-O2" "-Os" "-Oz" "-O3")
    case "$OSTYPE" in
        darwin*)  linking_modes=("0") ;;
        linux*)   linking_modes=("0" "1") ;;
        *)        echo "unknown: $OSTYPE"; exit 1 ;;
    esac
    execution_engines=("c" "i")
else
    optimization_modes=("-O0")
    case "$OSTYPE" in
        darwin*)  linking_modes=("0") ;;
        linux*)   linking_modes=("1") ;;
        *)        echo "unknown: $OSTYPE"; exit 1 ;;
    esac
    execution_engines=("i")
fi

for linking_mode in "${linking_modes[@]}"
do
    for optimization_mode in "${optimization_modes[@]}"
    do
        printf "\n===========================\n"
        echo "TESTS (STATIC=$linking_mode, $optimization_mode)"
        printf "===========================\n\n"

        exe_name="${base_exe_name}_${optimization_mode}_$linking_mode"
        compile $build_dir $exe_name $debug_flags $optimization_mode $linking_mode;

        printf "\n===========================\n"
        echo "COMPILER BUILD OK"
        printf "===========================\n\n"

        for test_name in "${test_names[@]}"
        do
            printf "\n===========================\n"
            echo "$test_name..."
            printf "===========================\n\n"

            for execution_engine in "${execution_engines[@]}"
            do
                cmd="build/$exe_name tests/$test_name.nat $execution_engine"
                echo "Run command: $cmd"
                eval "$cmd"
                printf "\n===========================\n"
                echo "$test_name [COMPILATION] [EXECUTION ENGINE: $execution_engine] [OK]"
                printf "===========================\n\n"

                if [ "$execution_engine" != "i" ]
                then
                    nest/$test_name
                fi

                printf "\n===========================\n"
                echo "$test_name [RUN] [OK]"
                printf "===========================\n\n"
            done
        done
    done
done

