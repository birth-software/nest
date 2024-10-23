#pragma once

#include <std/base.h>

typedef enum CompilerBackend : u8
{
    COMPILER_BACKEND_NEST,
    COMPILER_BACKEND_LLVM,
    COMPILER_BACKEND_COUNT,
} CompilerBackend;

fn String compiler_backend_to_string(CompilerBackend backend)
{
    switch (backend)
    {
    case COMPILER_BACKEND_NEST:
        return strlit("n");
    case COMPILER_BACKEND_LLVM:
        return strlit("l");
    case COMPILER_BACKEND_COUNT:
        unreachable();
    }
}

fn CompilerBackend string_to_compiler_backend(String string)
{
    CompilerBackend result = COMPILER_BACKEND_COUNT;

    for (u32 i = 0; i < COMPILER_BACKEND_COUNT; i += 1)
    {
        auto candidate = (CompilerBackend)i;
        if (s_equal(compiler_backend_to_string(candidate), string))
        {
            result = candidate;
            break;
        }
    }

    return result;
}

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
