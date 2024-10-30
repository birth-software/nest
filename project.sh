#!/usr/bin/env bash
set -ex
original_dir=$PWD
build_dir=build
C_COMPILER_PATH=clang
CXX_COMPILER_PATH=clang++
ASM_COMPILER_PATH=clang

case $BIRTH_OS in
    windows)
        C_COMPILER_OPT_ARG="-DCMAKE_C_COMPILER_TARGET=x86_64-pc-windows-msvc"
        CXX_COMPILER_OPT_ARG="-DCMAKE_CXX_COMPILER_TARGET=x86_64-pc-windows-msvc"
        ASM_COMPILER_OPT_ARG="-DCMAKE_ASM_COMPILER_TARGET=x86_64-pc-windows-msvc"
        ;;
    *)
        C_COMPILER_OPT_ARG=""
        CXX_COMPILER_OPT_ARG=""
        ASM_COMPILER_OPT_ARG=""
        ;;
esac

mkdir -p $build_dir
cmake . \
    -B$build_dir \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH \
    -DCMAKE_C_COMPILER=$C_COMPILER_PATH \
    -DCMAKE_CXX_COMPILER=$CXX_COMPILER_PATH \
    -DCMAKE_ASM_COMPILER=$ASM_COMPILER_PATH \
    $C_COMPILER_OPT_ARG \
    $CXX_COMPILER_OPT_ARG \
    $ASM_COMPILER_OPT_ARG
cd $build_dir
ninja -v
cd $original_dir

if [ "$#" -ne 0 ]; then
    $build_dir/runner $@
fi
