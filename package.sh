#!/usr/bin/env bash

set -eux

if [ "$BB_IS_CI" == "true" ]; then
    case "$BIRTH_OS" in
        windows)
            OPT_EXTENSION=".exe";;
        *)
            OPT_EXTENSION="";;
    esac

    BB_EXE_PATH="$BUILD_DIR/$COMPILER_NAME$OPT_EXTENSION"
    BB_INSTALL_NAME=bloat-buster-$BIRTH_ARCH-$BIRTH_OS-$CMAKE_BUILD_TYPE
    BB_INSTALL_PATH="$PWD/$BB_INSTALL_NAME"
    mkdir -p $BB_INSTALL_PATH
    cp $BB_EXE_PATH $BB_INSTALL_PATH
    7z a -t7z -m0=lzma2 -mx=9 -mfb=64 -md=64m -ms=on $BB_INSTALL_NAME.7z $BB_INSTALL_PATH
    b2sum $BB_INSTALL_NAME.7z > "$BB_INSTALL_NAME.7z.b2sum"

    case "$BIRTH_OS" in
        windows) BB_INSTALL_PATH="$(cygpath -w ${BB_INSTALL_PATH})" ;;
        *) ;;
    esac

    echo "BLOAT_BUSTER_RELEASE_NAME_BASE=$BB_INSTALL_NAME" >> $GITHUB_ENV
    echo "BLOAT_BUSTER_RELEASE_PATH_BASE=$BB_INSTALL_PATH" >> $GITHUB_ENV
fi
