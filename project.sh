#!/usr/bin/env bash
set -eux
original_dir=$PWD
build_dir=$original_dir/build
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

case "$BIRTH_OS" in
    linux)
        ls -las /
        ls -las /usr
        ls -las /usr/lib
        ;;
    *)
        ;;
esac

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
    CMAKE_PREFIX_PATH="$HOME/Downloads/llvm-$BIRTH_ARCH-$BIRTH_OS-$CMAKE_BUILD_TYPE"
fi

if [[ -z "${BUSTER_GITHUB_RUN-}" ]]; then
    BUSTER_GITHUB_RUN=false
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
case $BIRTH_OS in
    linux)
        USE_MOLD_OPT_ARG=-DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold"
        ;;
    *)
        USE_MOLD_OPT_ARG=""
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
    $USE_MOLD_OPT_ARG \
    $C_COMPILER_OPT_ARG \
    $CXX_COMPILER_OPT_ARG \
    $ASM_COMPILER_OPT_ARG
    
cd $build_dir
ninja -v
cd $original_dir

if [ "$#" -ne 0 ]; then
    $build_dir/runner $@
fi

if [ "$BUSTER_GITHUB_RUN" == "true" ]; then
    case "$BIRTH_OS" in
        windows)
            OPT_EXTENSION=".exe";;
        *)
            OPT_EXTENSION="";;
    esac

    BB_EXE_PATH="$build_dir/bb$OPT_EXTENSION"
    BB_INSTALL_NAME=bloat-buster-$BIRTH_ARCH-$BIRTH_OS-$CMAKE_BUILD_TYPE
    BB_INSTALL_PATH="$PWD/$BB_INSTALL_NAME"
    mkdir -p $BB_INSTALL_PATH
    cp $BB_EXE_PATH $BB_INSTALL_PATH
    7z a -t7z -m0=lzma2 -mx=9 -mfb=64 -md=64m -ms=on $BB_INSTALL_NAME.7z $BB_INSTALL_PATH
    b2sum $BB_INSTALL_NAME.7z > "$BB_INSTALL_NAME.7z.b2sum"

    case "$BIRTH_OS" in
        windows) BB_INSTALL_PATH="$(cygpath -w ${BB_INSTALL_PATH})" ;;
        *) ;;
    esac

    echo "BLOAT_BUSTER_RELEASE_PATH_BASE=$BB_INSTALL_PATH" >> $GITHUB_ENV
fi
