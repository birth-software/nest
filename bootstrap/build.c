#include "lib.h"

#define build_dir "build"
#define nest_dir "nest"

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

STRUCT(CompileOptions)
{
    char* out_path;
    char* in_path;
    OptimizationMode optimization_mode:3;
    u8 debug_info:1;
    u8 error_on_warning:1;
    Compiler compiler:1;
    Linkage linkage:1;
};
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
#ifdef __APPLE__
        compiler = "/opt/homebrew/opt/llvm/bin/clang";
#else
        compiler = "/usr/bin/clang";
#endif
        break;
    case COMPILER_COUNT:
        unreachable();
    }

    *vb_add(args, 1) = compiler;
    // *vb_add(args, 1) = "-E";
    *vb_add(args, 1) = options->in_path;
    *vb_add(args, 1) = "-o";
    *vb_add(args, 1) = options->out_path;

    if (options->debug_info)
    {
        *vb_add(args, 1) = "-g3";
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

    *vb_add(args, 1) = "-march=native";

    if (options->error_on_warning)
    {
        *vb_add(args, 1) = "-Werror";
    }

    char* general_options[] = {
        "-std=gnu2x",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wconversion",
        "-Wno-nested-anon-types",
        "-Wno-keyword-macro",
        "-Wno-gnu-auto-type",
        "-Wno-auto-decl-extensions",
        "-Wno-gnu-empty-initializer",
        "-Wno-fixed-enum-extension",
        "-Wno-gnu-binary-literal",
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
        char* static_options[] = { "-ffreestanding", "-nostdlib", "-static", "-DSTATIC", "-lgcc" };
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

typedef enum CompilerBackend
{
    COMPILER_BACKEND_INTERPRETER,
    COMPILER_BACKEND_C,
    COMPILER_BACKEND_MACHINE,
    COMPILER_BACKEND_COUNT,
} CompilerBackend;
declare_slice(CompilerBackend);

fn void compile_and_run(const CompileOptions* const options, char** envp, CompilerBackend compiler_backend, u8 debug, char* nest_source_path)
{
    compile_c(options, envp);
    CStringSlice args = {};
    char* compiler_backend_string;
    switch (compiler_backend)
    {
    case COMPILER_BACKEND_C:
        compiler_backend_string = "c";
        break;
    case COMPILER_BACKEND_INTERPRETER:
        compiler_backend_string = "i";
        break;
    case COMPILER_BACKEND_MACHINE:
        compiler_backend_string = "m";
        break;
    case COMPILER_BACKEND_COUNT:
        unreachable();
    }

#define common_compile_and_run_args \
                options->out_path, \
                nest_source_path, \
                compiler_backend_string, \
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
            "/usr/bin/lldb",
            "-o",
            "run",
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
    COMMAND_COMPILE,
    COMMAND_COUNT,
} Command;

STRUCT(TestOptions)
{
    Slice(Linkage) linkages;
    Slice(OptimizationMode) optimization_modes;
    Slice(String) test_paths;
    Slice(CompilerBackend) compiler_backends;
};

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
                strlit(build_dir "/" "nest"),
                strlit("_"),
                optimization_string,
                strlit("_"),
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
                auto test_dir = string_no_extension(test_path);
                auto test_name = string_base(test_dir);

                for (u32 engine_i = 0; engine_i < test_options->compiler_backends.length; engine_i += 1)
                {
                    CompilerBackend compiler_backend = test_options->compiler_backends.pointer[engine_i];
                    char* compiler_backend_string;
                    switch (compiler_backend)
                    {
                    case COMPILER_BACKEND_C:
                        compiler_backend_string = "c";
                        break;
                    case COMPILER_BACKEND_INTERPRETER:
                        compiler_backend_string = "i";
                        break;
                    case COMPILER_BACKEND_MACHINE:
                        compiler_backend_string = "m";
                        break;
                    case COMPILER_BACKEND_COUNT:
                        unreachable();
                    }

                    char* arguments[] = {
                        compile_options.out_path,
                        test_path_c,
                        compiler_backend_string,
                        0,
                    };

                    run_command((CStringSlice) array_to_slice(arguments), envp);

                    if (compiler_backend != COMPILER_BACKEND_INTERPRETER)
                    {
                        String out_program = arena_join_string(arena, ((Slice(String)) array_to_slice(((String[]){
                                            strlit(nest_dir "/"),
                                            test_name,
                                            }))));
                        char* run_arguments[] = {
                            string_to_c(out_program),
                            0,
                        };
                        run_command((CStringSlice) array_to_slice(run_arguments), envp);
                    }
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
    // calibrate_cpu_timer();

    CompilerBackend preferred_compiler_backend = COMPILER_BACKEND_COUNT;
    Command command = COMMAND_COUNT;
    u8 test_every_config = 0;
    String source_file_path = {};

    for (int i = 1; i < argc; i += 1)
    {
        char* c_argument = argv[i];
        auto argument = cstr(c_argument);
        if (s_equal(argument, strlit("i")))
        {
            preferred_compiler_backend = COMPILER_BACKEND_INTERPRETER;
        }
        else if (s_equal(argument, strlit("c")))
        {
            preferred_compiler_backend = COMPILER_BACKEND_C;
        }
        else if (s_equal(argument, strlit("m")))
        {
            preferred_compiler_backend = COMPILER_BACKEND_MACHINE;
        }
        else if (s_equal(argument, strlit("test")))
        {
            command = COMMAND_RUN_TESTS;
        }
        else if (s_equal(argument, strlit("debug")))
        {
            command = COMMAND_DEBUG;
        }
        else if (s_equal(argument, strlit("compile")))
        {
            command = COMMAND_COMPILE;
        }
        else if (s_equal(argument, strlit("all")))
        {
            test_every_config = 1;
        }
    }

    auto index = 2 - (command == COMMAND_COUNT);
    if (argc > index)
    {
        auto* c_argument = argv[index];
        auto argument = cstr(c_argument);
        String expected_starts[] = { strlit("tests/"), strlit("src/") };

        for (u32 i = 0; i < array_length(expected_starts); i += 1)
        {
            auto expected_start = expected_starts[i];
            if (expected_start.length < argument.length)
            {
                // TODO: make our own function
                if (strncmp(c_argument, string_to_c(expected_start), expected_start.length) == 0)
                {
                    source_file_path = argument;
                    break;
                }
            }
        }
    }

    if (command == COMMAND_COUNT && !source_file_path.pointer)
    {
        print("Expected a command\n");
        return 1;
    }

    if (command == COMMAND_COUNT)
    {
        command = COMMAND_COMPILE;
    }

    if ((command == COMMAND_DEBUG) | ((command == COMMAND_RUN_TESTS) & (test_every_config == 0)))
    {
        if (preferred_compiler_backend == COMPILER_BACKEND_COUNT)
        {
            preferred_compiler_backend = COMPILER_BACKEND_INTERPRETER;
        }
    }

    switch (command)
    {
    case COMMAND_DEBUG:
        if (!source_file_path.pointer)
        {
            fail();
        }

        Linkage linkage = 
#if defined(__linux__)
            LINKAGE_STATIC;
#else
            LINKAGE_DYNAMIC;
#endif

        compile_and_run(&(CompileOptions) {
            .in_path = compiler_source_path,
            .out_path = linkage == LINKAGE_DYNAMIC ? (build_dir "/" "nest_O0_dynamic") : (build_dir "/" "nest_O0_static"),
            .compiler = default_compiler,
            .debug_info = 1,
            .error_on_warning = 0,
            .optimization_mode = O0,
            .linkage = linkage,
        }, envp, preferred_compiler_backend, 1, string_to_c(source_file_path));
        break;
    case COMMAND_RUN_TESTS:
        {
            Arena* arena = arena_init_default(KB(64));
            Linkage all_linkages[] = { LINKAGE_DYNAMIC, LINKAGE_STATIC };
            static_assert(array_length(all_linkages) == LINKAGE_COUNT);
            OptimizationMode all_optimization_modes[] = {
                O0,
                O1,
                O2,
                O3,
                Os,
                Oz
            };
            // static_assert(array_length(all_optimization_modes) == OPTIMIZATION_COUNT);
            String every_single_test[] = {
                 strlit("tests/first.nat"),
                 // strlit("tests/add_sub.nat"),
                 // strlit("tests/mul.nat"),
                 // strlit("tests/div.nat"),
                 // strlit("tests/and.nat"),
                 // strlit("tests/or.nat"),
                 // strlit("tests/xor.nat"),
                 // strlit("tests/return_var.nat"),
                 // strlit("tests/return_mod_scope.nat"),
                 // strlit("tests/shift_left.nat"),
                 // strlit("tests/shift_right.nat"),
                 // strlit("tests/thousand_simple_functions.nat"),
                 // strlit("tests/simple_arg.nat"),
                 // strlit("tests/comparison.nat"),
            };
            CompilerBackend all_compiler_backends[] = {
                // COMPILER_BACKEND_INTERPRETER,
                // COMPILER_BACKEND_C,
#ifdef __linux__
                COMPILER_BACKEND_MACHINE,
#endif
            };

            Slice(Linkage) linkage_selection;
            Slice(OptimizationMode) optimization_selection;
            Slice(CompilerBackend) compiler_backend_selection;

            if (test_every_config)
            {
#ifdef __linux__
                linkage_selection = (Slice(Linkage)) array_to_slice(all_linkages);
#else 
                linkage_selection = (Slice(Linkage)) { .pointer = &all_linkages[0], .length = 1 };
#endif
                optimization_selection = (Slice(OptimizationMode)) array_to_slice(all_optimization_modes);
                compiler_backend_selection = (Slice(CompilerBackend)) array_to_slice(all_compiler_backends);
            }
            else
            {
                linkage_selection = (Slice(Linkage)) { .pointer = &all_linkages[0], .length = 1 };
                optimization_selection = (Slice(OptimizationMode)) { .pointer = &all_optimization_modes[0], .length = 1 };
                compiler_backend_selection = (Slice(CompilerBackend)) { .pointer = &preferred_compiler_backend, .length = 1 };
            }

            Slice(String) test_selection;
            if (source_file_path.pointer)
            {
                test_selection = (Slice(String)) { .pointer = &source_file_path, .length = 1 };
            }
            else
            {
                test_selection = (Slice(String)) array_to_slice(every_single_test);
            }

            run_tests(arena, &(TestOptions) {
                .linkages = linkage_selection,
                .optimization_modes = optimization_selection,
                .test_paths = test_selection,
                .compiler_backends = compiler_backend_selection,
            }, envp);
        } break;
    case COMMAND_COMPILE:
        compile_c(&(CompileOptions) {
            .in_path = compiler_source_path,
            .out_path = build_dir "/" "nest_O0_static",
            .compiler = default_compiler,
            .debug_info = 1,
            .error_on_warning = 0,
            .optimization_mode = O0,
#if defined(__linux__)
            .linkage = LINKAGE_STATIC,
#else
            .linkage = LINKAGE_DYNAMIC,
#endif
        }, envp);
        break;
    case COMMAND_COUNT:
        unreachable();
    }
}
