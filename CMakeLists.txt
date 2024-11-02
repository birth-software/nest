cmake_minimum_required(VERSION 3.10)
project(bloat-buster)

include(CMakePrintHelpers)
cmake_print_variables("Build type: ${CMAKE_BUILD_TYPE}")
cmake_print_variables("C flags Debug: ${CMAKE_C_FLAGS_DEBUG}")
cmake_print_variables("CXX flags Debug: ${CMAKE_CXX_FLAGS_DEBUG}")
cmake_print_variables("C flags MinSizeRel: ${CMAKE_C_FLAGS_MINSIZEREL}")
cmake_print_variables("CXX flags MinSizeRel: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
cmake_print_variables("C flags RelWithDebInfo: ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
cmake_print_variables("CXX flags RelWithDebInfo: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
cmake_print_variables("C flags Release: ${CMAKE_C_FLAGS_RELEASE}")
cmake_print_variables("CXX flags Release: ${CMAKE_CXX_FLAGS_RELEASE}")

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

find_package(LLVM REQUIRED CONFIG)

find_program(LLVM_CONFIG_EXE
    NAMES llvm-config-19 llvm-config-19.0 llvm-config190 llvm-config19 llvm-config NAMES_PER_DIR
    PATHS
    "/mingw64/bin"
    "/c/msys64/mingw64/bin"
    "c:/msys64/mingw64/bin"
    "C:/Libraries/llvm-19.0.0/bin")

if ("${LLVM_CONFIG_EXE}" STREQUAL "LLVM_CONFIG_EXE-NOTFOUND")
    if (NOT LLVM_CONFIG_ERROR_MESSAGES STREQUAL "")
        list(JOIN LLVM_CONFIG_ERROR_MESSAGES "\n" LLVM_CONFIG_ERROR_MESSAGE)
        message(FATAL_ERROR ${LLVM_CONFIG_ERROR_MESSAGE})
    else()
        message(FATAL_ERROR "unable to find llvm-config")
    endif()
endif()


execute_process(
    COMMAND ${LLVM_CONFIG_EXE} --libs
    OUTPUT_VARIABLE LLVM_LIBRARIES_SPACES
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE " " ";" LLVM_LIBRARIES "${LLVM_LIBRARIES_SPACES}")

execute_process(
    COMMAND ${LLVM_CONFIG_EXE} --libdir
    OUTPUT_VARIABLE LLVM_LIBDIRS_SPACES
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE " " ";" LLVM_LIBDIRS "${LLVM_LIBDIRS_SPACES}")

execute_process(
    COMMAND ${LLVM_CONFIG_EXE} --system-libs
    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS_SPACES
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE " " ";" LLVM_SYSTEM_LIBS "${LLVM_SYSTEM_LIBS_SPACES}")

execute_process(
    COMMAND ${LLVM_CONFIG_EXE} --shared-mode
    OUTPUT_VARIABLE LLVM_LINK_MODE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if (${LLVM_LINK_MODE} STREQUAL "shared")
    # We always ask for the system libs corresponding to static linking,
    # since on some distros LLD is only available as a static library
    # and we need these libraries to link it successfully
    execute_process(
        COMMAND ${LLVM_CONFIG_EXE} --system-libs --link-static
        OUTPUT_VARIABLE LLVM_STATIC_SYSTEM_LIBS_SPACES
        ERROR_QUIET # Some installations have no static libs, we just ignore the failure
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" LLVM_STATIC_SYSTEM_LIBS "${LLVM_STATIC_SYSTEM_LIBS_SPACES}")

    set(LLVM_LIBRARIES ${LLVM_LIBRARIES} ${LLVM_SYSTEM_LIBS} ${LLVM_STATIC_SYSTEM_LIBS})
else()
    set(LLVM_LIBRARIES ${LLVM_LIBRARIES} ${LLVM_SYSTEM_LIBS})
endif()

execute_process(
    COMMAND ${LLVM_CONFIG_EXE} --includedir
    OUTPUT_VARIABLE LLVM_INCLUDE_DIRS_SPACES
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE " " ";" LLVM_INCLUDE_DIRS "${LLVM_INCLUDE_DIRS_SPACES}")

if (APPLE)
    if (MSVC)
        list(REMOVE_ITEM LLVM_LIBRARIES "zstd.lib")
    else()
        list(REMOVE_ITEM LLVM_LIBRARIES "-lzstd")
    endif()

    find_library(ZSTD NAMES libzstd.a libzstdstatic.a zstd NAMES_PER_DIR)
    list(APPEND LLVM_LIBRARIES "${ZSTD}")
endif()

link_directories("${LLVM_LIBDIRS}")

set(LIBRARY_NAME "std")
set(RUNNER_NAME "runner")
set(COMPILER_NAME "bb")

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
target_link_libraries(${RUNNER_NAME} PRIVATE ${LIBRARY_NAME})

add_executable("${COMPILER_NAME}"
    "bootstrap/bloat-buster/main.c"
    "bootstrap/bloat-buster/pdb_image.c"
    "bootstrap/bloat-buster/llvm.cpp"
)

target_compile_definitions(${COMPILER_NAME} PRIVATE ${LLVM_DEFINITIONS})
cmake_print_variables("LLVM definitions: ${LLVM_DEFINITIONS}")
target_include_directories(${COMPILER_NAME} PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_directories(${COMPILER_NAME} PRIVATE ${LLVM_LIBRARY_DIRS})
target_link_libraries(${COMPILER_NAME} PRIVATE ${LIBRARY_NAME} ${LLVM_LIBRARIES})

