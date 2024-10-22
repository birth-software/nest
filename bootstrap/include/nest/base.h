#pragma once

#include <std/base.h>

typedef enum CompilerBackend : u8
{
    COMPILER_BACKEND_NEST = 'm',
    // COMPILER_BACKEND_LLVM,
    COMPILER_BACKEND_COUNT,
} CompilerBackend;

typedef enum CpuArchitecture : u8
{
    CPU_ARCH_X86_64,
    CPU_ARCH_AARCH64,
} CpuArchitecture;

typedef enum OperatingSystem : u8
{
    OPERATING_SYSTEM_LINUX,
    OPERATING_SYSTEM_MAC,
    OPERATING_SYSTEM_WINDOWS,
} OperatingSystem;

STRUCT(Target)
{
    CpuArchitecture cpu;
    OperatingSystem os;
};

STRUCT(CodegenOptions)
{
    String test_name;
    Target target;
    CompilerBackend backend;
    u8 generate_debug_information;
};
