#!/usr/bin/env bash
set -eux

LLVM_VERSION=19.1.0
FILENAME_BASE=llvm-$BIRTH_ARCH-$BIRTH_OS-$CMAKE_BUILD_TYPE
ZIP_NAME=$FILENAME_BASE.7z
BASE_URL=https://github.com/birth-software/llvm/releases/download/v$LLVM_VERSION/$ZIP_NAME
wget --version 1>/dev/null 2>/dev/null || choco install wget
wget $BASE_URL
wget $BASE_URL.b2sum
b2sum -c $ZIP_NAME.b2sum
7z x $ZIP_NAME
CMAKE_PREFIX_PATH_INTERNAL=$PWD/$FILENAME_BASE
if [[ -n "${GITHUB_RUN_ID-}" ]]; then
    echo "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH_INTERNAL" >> $GITHUB_OUTPUT
else
    export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH_INTERNAL
fi
