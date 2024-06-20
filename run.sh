#!/bin/sh
set -ex
mkdir -p build
time clang++ \
-o build/hatch \
bootstrap/main.cpp \
bootstrap/entry.S \
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
-MJ build/compile_commands.json
gf2 -ex r build/hatch
