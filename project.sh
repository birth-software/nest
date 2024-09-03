#!/bin/bash
set -e
mkdir -p build
time clang -o build/build bootstrap/build.c -g -march=native -std=gnu2x -Wall -Wextra -Wpedantic -Wno-nested-anon-types -Wno-keyword-macro -Wno-gnu-auto-type -Wno-auto-decl-extensions -Wno-gnu-empty-initializer -Wno-fixed-enum-extension -pedantic -fno-exceptions -fno-stack-protector
build/build $@
