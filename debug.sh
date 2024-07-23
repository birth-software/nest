#!/bin/bash

set -e

path=$1
echo $path

source ./compile.sh
build_dir="build"
exe_name="nest"
exe_path=$build_dir/$exe_name
debug_flags="-g"
optimization_flags=""
bootstrap_args="$path"

compile $build_dir $exe_name $debug_flags $optimization_flags

case "$OSTYPE" in
    darwin*)  lldb -- $exe_path $bootstrap_args;; 
    linux*)   gf2 -ex r --args $exe_path $bootstrap_args;;
    *)        echo "unknown: $OSTYPE" ;;
esac
