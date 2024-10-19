cmake_minimum_required(VERSION 3.10)
project(nest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
add_compile_options(
    -pedantic
    -Wall -Wextra -Wpedantic
    -Wno-nested-anon-types -Wno-keyword-macro -Wno-gnu-auto-type -Wno-auto-decl-extensions -Wno-gnu-empty-initializer -Wno-fixed-enum-extension -Wno-gnu-binary-literal
    -fno-exceptions -fno-stack-protector
    -fdiagnostics-color=always -ferror-limit=1
    -march=native
)
include_directories("bootstrap/include")

set(LIBRARY_NAME "std")
set(RUNNER_NAME "runner")
set(COMPILER_NAME "nest")
add_library("${LIBRARY_NAME}"
    "bootstrap/std/base.c"
    "bootstrap/std/string.c"
    "bootstrap/std/os.c"
    "bootstrap/std/entry_point.c"
    "bootstrap/std/virtual_buffer.c"
    "bootstrap/std/md5.c"
    "bootstrap/std/sha1.c"
)
add_executable("${RUNNER_NAME}" "bootstrap/runner/runner.c")
target_link_libraries(${RUNNER_NAME} ${LIBRARY_NAME})

add_executable("${COMPILER_NAME}"
    "bootstrap/src/main.c"
    "bootstrap/src/pdb_image.c"
)
target_link_libraries(${COMPILER_NAME} ${LIBRARY_NAME})
