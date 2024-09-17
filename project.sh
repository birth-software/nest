#!/bin/bash
set -ex
mkdir -p build
case "$OSTYPE" in
  linux*)  CLANG_PATH="clang" ;; 
  darwin*)   CLANG_PATH="/opt/homebrew/opt/llvm/bin/clang" ;;
  *)        exit 1 ;;
esac
time $CLANG_PATH -o build/build bootstrap/build.c -g -march=native -std=gnu2x -Wall -Wextra -Wpedantic -Wno-nested-anon-types -Wno-keyword-macro -Wno-gnu-auto-type -Wno-auto-decl-extensions -Wno-gnu-empty-initializer -Wno-fixed-enum-extension -pedantic -fno-exceptions -fno-stack-protector
build/build $@
