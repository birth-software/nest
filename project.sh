#!/usr/bin/env bash
set -ex
release_mode="Debug"
if [[ "$1" =~ ^build_type=.* ]]; then
    release_mode=${1#build_type=}
fi
echo "Build type: $release_mode"
build_dir=build
mkdir -p $build_dir

case "$OSTYPE" in
    linux*) CLANG_PREFIX="/usr/bin" ;; 
    darwin*) CLANG_PREFIX="/opt/homebrew/opt/llvm/bin" ;;
    *)        exit 1 ;;
esac

case "$OSTYPE" in
    linux*) cmake . -B$build_dir -G Ninja -DCMAKE_BUILD_TYPE="$release_mode" -DCMAKE_C_COMPILER="$CLANG_PREFIX/clang" -DCMAKE_CXX_COMPILER="$CLANG_PREFIX/clang++" -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=mold" ;;
    darwin*) cmake . -B$build_dir -G Ninja -DCMAKE_BUILD_TYPE="$release_mode" -DCMAKE_C_COMPILER="$CLANG_PREFIX/clang" -DCMAKE_CXX_COMPILER="$CLANG_PREFIX/clang++" "-DCMAKE_PREFIX_PATH=$(brew --prefix zstd);$(brew --prefix llvm)" ;;
    *)        exit 1 ;;
esac

original_dir=$PWD
cd $build_dir
ninja
cd $original_dir

if [ "$#" -ne 0 ]; then
    $build_dir/runner $@
fi
