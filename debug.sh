#!/bin/bash

set -e

path=$1
if [ "$path" == "" ]
then
    echo "error: a valid binary path must be provided to debug"
    exit 1
fi

source ./compile.sh
build_dir="build"
exe_name="nest"
exe_path=$build_dir/$exe_name
debug_flags="-g"
optimization_flags="-O0"
bootstrap_args="$path"
case "$OSTYPE" in
    darwin*)  static=0;; 
    linux*)   static=1;;
    *)        echo "unknown: $OSTYPE" ;;
esac

compile $build_dir $exe_name $debug_flags $optimization_flags $static

case "$OSTYPE" in
    darwin*)  lldb -- $exe_path $bootstrap_args;; 
    linux*)   gf2 -ex b entry_point -ex r --args $exe_path $bootstrap_args;;
    *)        echo "unknown: $OSTYPE" ;;
esac
