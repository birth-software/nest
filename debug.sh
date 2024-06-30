#!/bin/bash
set -ex
source ./compile.sh
gf2 -ex r $NEST_BUILD_DIR/$NEST_EXE_NAME
