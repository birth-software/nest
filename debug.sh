#!/bin/bash

set -e

source ./compile.sh
build_dir="build"
exe_name="nest"
exe_path=$build_dir/$exe_name

compile $build_dir $exe_name "-g" "";

case "$OSTYPE" in
    darwin*)  lldb $exe_path ;; 
    linux*)   gf2 -ex r $exe_path ;;
    *)        echo "unknown: $OSTYPE" ;;
esac
