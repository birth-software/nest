#!/bin/sh
NEST_BUILD_DIR=build
NEST_EXE_NAME=nest
set -ex
mkdir -p $NEST_BUILD_DIR
time clang++ \
-o $NEST_BUILD_DIR/$NEST_EXE_NAME \
bootstrap/main.cpp \
bootstrap/entry.S \
`# -Oz` \
`# -march=native` \
`# -Wl,-strip-all` \
-g \
-std=gnu++23 \
-Wall \
-Wextra \
-Wpedantic \
-Wno-nested-anon-types \
-pedantic \
-ffreestanding \
-nostdlib \
-static \
-fno-exceptions \
-fno-stack-protector \
-ferror-limit=1 \
`#-ftime-report` \
-MJ $NEST_BUILD_DIR/compile_commands.json
