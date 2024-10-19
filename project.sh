#!/usr/bin/env bash
set -ex
release_mode="Debug"
if [[ "$1" =~ ^build_type=.* ]]; then
    release_mode=${1#build_type=}
fi
echo "Build type: $release_mode"
build_dir=build
mkdir -p $build_dir
cmake . -B$build_dir -G Ninja -DCMAKE_BUILD_TYPE="$release_mode" -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++"
original_dir=$PWD
cd $build_dir
ninja
cd $original_dir

if [ "$#" -ne 0 ]; then
    $build_dir/runner $@
fi
