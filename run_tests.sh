#!/bin/bash

set -ex
source ./compile.sh
compile "build" "nest" "-g" "";
build/nest
