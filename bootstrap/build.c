#include "lib.h"

#define build_dir "build"

typedef enum OptimizationMode : u8
{
    O0,
    O1,
    O2,
    O3,
    Os,
    Oz,
    OPTIMIZATION_COUNT,
} OptimizationMode;
declare_slice(OptimizationMode);

typedef enum Compiler : u8
{
    gcc,
    clang,
    COMPILER_COUNT,
} Compiler;
declare_slice(Compiler);

global const Compiler default_compiler = clang;

typedef enum Linkage: u8
{
    LINKAGE_DYNAMIC,
    LINKAGE_STATIC,
    LINKAGE_COUNT,
} Linkage;
declare_slice(Linkage);

struct CompileOptions
{
    char* out_path;
    char* in_path;
    OptimizationMode optimization_mode:3;
    u8 debug_info:1;
    u8 error_on_warning:1;
    Compiler compiler:1;
    Linkage linkage:1;
};
typedef struct CompileOptions CompileOptions;
decl_vbp(char);

fn u8 is_debug(OptimizationMode optimization_mode, u8 debug_info)
{
    return (optimization_mode == O0) & (debug_info != 0);
}

fn void compile_c(const CompileOptions *const options, char** envp)
{
    VirtualBufferP(char) argument_stack = {};
    auto* args = &argument_stack;
    char* compiler;

    switch (options->compiler)
    {
    case gcc:
        compiler = "/usr/bin/gcc";
        break;
    case clang:
        compiler = "/usr/bin/clang";
        break;
    case COMPILER_COUNT:
        unreachable();
    }

    *vb_add(args, 1) = compiler;
    *vb_add(args, 1) = options->in_path;
    *vb_add(args, 1) = "-o";
    *vb_add(args, 1) = options->out_path;

    if (options->debug_info)
    {
        *vb_add(args, 1) = "-g";
    }

    switch (options->optimization_mode)
    {
    case O0:
        *vb_add(args, 1) = "-O0";
        break;
    case O1:
        *vb_add(args, 1) = "-O1";
        break;
    case O2:
        *vb_add(args, 1) = "-O2";
        break;
    case O3:
        *vb_add(args, 1) = "-O3";
        break;
    case Os:
        *vb_add(args, 1) = "-Os";
        break;
    case Oz:
        *vb_add(args, 1) = "-Oz";
        break;
    case OPTIMIZATION_COUNT:
        unreachable();
    }

    if (options->error_on_warning)
    {
        *vb_add(args, 1) = "-Werror";
    }

    char* general_options[] = {
        "-std=gnu2x",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wno-nested-anon-types",
        "-Wno-keyword-macro",
        "-Wno-gnu-auto-type",
        "-Wno-auto-decl-extensions",
        "-Wno-gnu-empty-initializer",
        "-Wno-fixed-enum-extension",
        "-pedantic",
        "-fno-exceptions",
        "-fno-stack-protector",
        "-ferror-limit=1",
    };
    memcpy(vb_add(args, array_length(general_options)), general_options, sizeof(general_options));

    if (!is_debug(options->optimization_mode, options->debug_info))
    {
        *vb_add(args, 1) = "-DNDEBUG=1";
    }

    if (options->linkage == LINKAGE_STATIC)
    {
        char* static_options[] = { "-ffreestanding", "-nostdlib", "-static", "-DSTATIC", };
        memcpy(vb_add(args, array_length(static_options)), static_options, sizeof(static_options));
    }

    if (options->compiler == clang)
    {
        *vb_add(args, 1) = "-MJ";
        *vb_add(args, 1) = build_dir "/" "compile_commands.json";
    }

    *vb_add(args, 1) = 0;

    run_command((CStringSlice) { .pointer = args->pointer, .length = args->length }, envp);
}

typedef enum ExecutionEngine
{
    EXECUTION_ENGINE_INTERPRETER,
    EXECUTION_ENGINE_C,
    EXECUTION_ENGINE_COUNT,
} ExecutionEngine;
declare_slice(ExecutionEngine);

fn void compile_and_run(const CompileOptions* const options, char** envp, ExecutionEngine execution_engine, u8 debug, char* nest_source_path)
{
    compile_c(options, envp);
    CStringSlice args = {};
    char* execution_engine_string;
    switch (execution_engine)
    {
    case EXECUTION_ENGINE_C:
        execution_engine_string = "c";
        break;
    case EXECUTION_ENGINE_INTERPRETER:
        execution_engine_string = "i";
        break;
    case EXECUTION_ENGINE_COUNT:
        unreachable();
    }

#define common_compile_and_run_args \
                options->out_path, \
                nest_source_path, \
                execution_engine_string, \
                0,

    if (debug)
    {
#ifdef __linux__
        args = (CStringSlice) array_to_slice(((char*[]){ 
            "/home/david/source/gf/gf2",
            "-ex",
            "set auto-solib-add off",
            "-ex",
            "r",
            "--args",
            common_compile_and_run_args 
        }));
#elif defined(__APPLE__)
        args = (CStringSlice) array_to_slice(((char*[]){ 
            "lldb",
            "--",
            common_compile_and_run_args 
        }));
#endif
    }
    else
    {
        args = (CStringSlice) array_to_slice(((char*[]){
            common_compile_and_run_args
        }));
    }

    run_command(args, envp);
}

typedef enum Command : u8
{
    COMMAND_DEBUG,
    COMMAND_RUN_TESTS,
    COMMAND_COUNT,
} Command;

struct TestOptions
{
    Slice(Linkage) linkages;
    Slice(OptimizationMode) optimization_modes;
    Slice(String) test_paths;
    Slice(ExecutionEngine) execution_engines;
};
typedef struct TestOptions TestOptions;

fn String linkage_name(Linkage linkage)
{
    switch (linkage)
    {
    case LINKAGE_STATIC:
        return strlit("static");
    case LINKAGE_DYNAMIC:
        return strlit("dynamic");
    case LINKAGE_COUNT:
        unreachable();
    }
}

fn String optimization_name(OptimizationMode optimization_mode)
{
    switch (optimization_mode)
    {
    case O0:
        return strlit("O0");
    case O1:
        return strlit("O1");
    case O2:
        return strlit("O2");
    case O3:
        return strlit("O3");
    case Os:
        return strlit("Os");
    case Oz:
        return strlit("Oz");
    case OPTIMIZATION_COUNT:
        unreachable();
    }
}

global const auto compiler_source_path = "bootstrap/main.c";

fn void run_tests(Arena* arena, TestOptions const * const test_options, char** envp)
{
    CompileOptions compile_options = {};
    compile_options.compiler = default_compiler;
    compile_options.debug_info = 1;
    compile_options.in_path = compiler_source_path;

    for (u32 linkage_i = 0; linkage_i < test_options->linkages.length; linkage_i += 1)
    {
        compile_options.linkage = test_options->linkages.pointer[linkage_i];
        auto linkage_string = linkage_name(compile_options.linkage);

        for (u32 optimization_i = 0; optimization_i < test_options->optimization_modes.length; optimization_i += 1)
        {
            compile_options.optimization_mode = test_options->optimization_modes.pointer[optimization_i];
            auto optimization_string = optimization_name(compile_options.optimization_mode);

            print("\n===========================\n");
            print("TESTS (linkage={s}, optimization={s})\n", linkage_string, optimization_string);
            print("===========================\n\n");

            String compiler_path = arena_join_string(arena, ((Slice(String)) array_to_slice(((String[]){
                strlit(build_dir "/"),
                strlit("nest"),
                optimization_string,
                linkage_string,
            }))));
            compile_options.out_path = string_to_c(compiler_path);

            compile_c(&compile_options, envp);

            print("\n===========================\n");
            print("COMPILER BUILD [OK]\n");
            print("===========================\n\n");

            for (u32 test_i = 0; test_i < test_options->test_paths.length; test_i += 1)
            {
                String test_path = test_options->test_paths.pointer[test_i];
                char* test_path_c = string_to_c(test_path);

                for (u32 engine_i = 0; engine_i < test_options->execution_engines.length; engine_i += 1)
                {
                    ExecutionEngine execution_engine = test_options->execution_engines.pointer[engine_i];
                    char* execution_engine_arg;
                    switch (execution_engine)
                    {
                    case EXECUTION_ENGINE_C:
                        execution_engine_arg = "c";
                        break;
                    case EXECUTION_ENGINE_INTERPRETER:
                        execution_engine_arg = "i";
                        break;
                    case EXECUTION_ENGINE_COUNT:
                        unreachable();
                    }

                    char* arguments[] = {
                        compile_options.out_path,
                        test_path_c,
                        execution_engine_arg,
                        0,
                    };

                    run_command((CStringSlice) array_to_slice(arguments), envp);
                }
            }
        }
    }
}

int main(int argc, char* argv[], char** envp)
{
    if (argc < 2)
    {
        print("Expected some arguments\n");
        return 1;
    }

    ExecutionEngine preferred_execution_engine = EXECUTION_ENGINE_COUNT;
    Command command = COMMAND_COUNT;
    u8 test_every_config = 0;

    for (int i = 1; i < argc; i += 1)
    {
        char* c_argument = argv[i];
        auto argument = cstr(c_argument);
        if (s_equal(argument, strlit("i")))
        {
            preferred_execution_engine = EXECUTION_ENGINE_INTERPRETER;
        }
        else if (s_equal(argument, strlit("c")))
        {
            preferred_execution_engine = EXECUTION_ENGINE_C;
        }
        else if (s_equal(argument, strlit("test")))
        {
            command = COMMAND_RUN_TESTS;
        }
        else if (s_equal(argument, strlit("debug")))
        {
            command = COMMAND_DEBUG;
        }
        else if (s_equal(argument, strlit("all")))
        {
            test_every_config = 1;
        }
    }

    if (command == COMMAND_COUNT)
    {
        print("Expected a command\n");
        return 1;
    }

    if ((command == COMMAND_DEBUG) | ((command == COMMAND_RUN_TESTS) & (test_every_config == 0)))
    {
        preferred_execution_engine = EXECUTION_ENGINE_INTERPRETER;
    }

    switch (command)
    {
    case COMMAND_DEBUG:
        compile_and_run(&(CompileOptions) {
            .in_path = compiler_source_path,
            .out_path = build_dir "/" "nest",
            .compiler = default_compiler,
            .debug_info = 1,
            .error_on_warning = 0,
            .optimization_mode = O0,
            .linkage = LINKAGE_STATIC,
        }, envp, EXECUTION_ENGINE_INTERPRETER, 1, "tests/first.nat");
        break;
    case COMMAND_RUN_TESTS:
        {
            Arena* arena = arena_init_default(KB(64));
            Linkage all_linkages[] = { LINKAGE_DYNAMIC, LINKAGE_STATIC };
            static_assert(array_length(all_linkages) == LINKAGE_COUNT);
            OptimizationMode all_optimization_modes[] = { O0, O1, O2, O3, Os, Oz };
            static_assert(array_length(all_optimization_modes) == OPTIMIZATION_COUNT);
            String all_test_paths[] = {
                 strlit("tests/first.nat"),
            };
            ExecutionEngine all_execution_engines[] = { EXECUTION_ENGINE_INTERPRETER, EXECUTION_ENGINE_C };
            static_assert(array_length(all_execution_engines) == EXECUTION_ENGINE_COUNT);

            Slice(String) test_selection = (Slice(String)) array_to_slice(all_test_paths);
            Slice(Linkage) linkage_selection;
            Slice(OptimizationMode) optimization_selection;
            Slice(ExecutionEngine) execution_engine_selection;

            if (test_every_config)
            {
#ifdef __linux__
                linkage_selection = (Slice(Linkage)) array_to_slice(all_linkages);
#else 
                linkage_selection = (Slice(Linkage)) { .pointer = &all_linkages[0], .length = 1 };
#endif
                optimization_selection = (Slice(OptimizationMode)) array_to_slice(all_optimization_modes);
                execution_engine_selection = (Slice(ExecutionEngine)) array_to_slice(all_execution_engines);
            }
            else
            {
                linkage_selection = (Slice(Linkage)) { .pointer = &all_linkages[0], .length = 1 };
                optimization_selection = (Slice(OptimizationMode)) { .pointer = &all_optimization_modes[0], .length = 1 };
                execution_engine_selection = (Slice(ExecutionEngine)) { .pointer = &preferred_execution_engine, .length = 1 };
            }

            run_tests(arena, &(TestOptions) {
                .linkages = linkage_selection,
                .optimization_modes = optimization_selection,
                .test_paths = test_selection,
                .execution_engines = execution_engine_selection,
            }, envp);
        } break;
    case COMMAND_COUNT:
        unreachable();
    }
}
