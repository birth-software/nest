#!/usr/bin/env bash
set -eux
original_dir=$PWD
build_dir=build
C_COMPILER_PATH=clang
CXX_COMPILER_PATH=clang++
ASM_COMPILER_PATH=clang

if [[ -z "${BIRTH_OS-}" ]]; then
    case "$OSTYPE" in
        msys*)
            BIRTH_OS=windows
            ;;
        linux*)
            BIRTH_OS=linux
            ;;
        darwin*)
            BIRTH_OS=macos
            ;;
        *)
            exit 1
            ;;
    esac
fi

if [[ -z "${BIRTH_ARCH-}" ]]; then
    case "$(uname -m)" in
        x86_64)
            BIRTH_ARCH=x86_64
            ;;
        arm64)
            BIRTH_ARCH=aarch64
            ;;
        *)
            exit 1
            ;;
    esac
fi

if [[ -z "${CMAKE_BUILD_TYPE-}" ]]; then
    CMAKE_BUILD_TYPE=Debug
fi

if [[ -z "${CMAKE_PREFIX_PATH-}" ]]; then
    CMAKE_PREFIX_PATH=""
fi



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
