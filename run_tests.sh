#!/bin/bash

set -e
source ./compile.sh
compile "build" "nest" "-g" "";
build/nest
