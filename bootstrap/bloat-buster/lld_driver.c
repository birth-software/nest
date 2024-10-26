#include <bloat-buster/lld_driver.h>
#include <std/virtual_buffer.h>
#include <std/string.h>

fn String linux_crt_find_path()
{
    auto flags = (OSFileOpenFlags) {
        .read = 1,
    };
    auto permissions = (OSFilePermissions) {
        .readable = 1,
        .writable = 1,
    };
    if (os_file_descriptor_is_valid(os_file_open(strlit("/usr/lib/crti.o"), flags, permissions)))
    {
        return strlit("/usr/lib");
    }

    if (os_file_descriptor_is_valid(os_file_open(strlit("/usr/lib/x86_64-linux-gnu/crti.o"), flags, permissions)))
    {
        return strlit("/usr/lib/x86_64-linux-gnu");
    }

    todo();
}

fn String windows_msvc_find_path()
{
    auto flags = (OSFileOpenFlags) {
        .read = 1,
        .directory = 1,
    };
    auto permissions = (OSFilePermissions) {
        .readable = 1,
    };
    String possibilities[] = {
        strlit("C:/Program Files/Microsoft Visual Studio/2022/Enterprise"),
        strlit("C:/Program Files/Microsoft Visual Studio/2022/Community"),
    };
    for (u64 i = 0; i < array_length(possibilities); i += 1)
    {
        auto possibility = possibilities[i];
        auto fd = os_file_open(possibility, flags, permissions);

        if (os_file_descriptor_is_valid(fd))
        {
            return possibility;
        }
    }

    failed_execution();
}

fn void linux_add_crt_item(Arena* arena, VirtualBufferP(char)* args, String crt_path, String item)
{
    String parts[] = {
        crt_path,
        strlit("/"),
        item,
    };
    *vb_add(args, 1) = string_to_c(arena_join_string(arena, (Slice(String))array_to_slice(parts)));
}

SliceP(char) lld_driver(Arena* arena, LinkerArguments arguments)
{
    VirtualBufferP(char) args = {};

    char* driver;
    switch (arguments.target.os)
    {
    case OPERATING_SYSTEM_LINUX:
        driver = "ld.lld";
        break;
    case OPERATING_SYSTEM_MAC:
        driver = "ld64.lld";
        break;
    case OPERATING_SYSTEM_WINDOWS:
        driver = "lld-link";
        break;
    }
    *vb_add(&args, 1) = driver;

    if (arguments.target.os != OPERATING_SYSTEM_WINDOWS)
    {
        *vb_add(&args, 1) = "--error-limit=0";
    }

    switch (arguments.target.os)
    {
        case OPERATING_SYSTEM_WINDOWS:
            {
                String parts[] = {
                    strlit("-out:"),
                    arguments.out_path,
                };
                auto arg = arena_join_string(arena, (Slice(String))array_to_slice(parts));
                *vb_add(&args, 1) = string_to_c(arg);
            } break;
        default:
            {
                *vb_add(&args, 1) = "-o";
                *vb_add(&args, 1) = string_to_c(arguments.out_path);
            } break;
    }

    if (arguments.target.os != OPERATING_SYSTEM_WINDOWS)
    {
        for (u64 i = 0; i < arguments.objects.length; i += 1)
        {
            *vb_add(&args, 1) = string_to_c(arguments.objects.pointer[i]);
        }
    }

    switch (arguments.target.os)
    {
    case OPERATING_SYSTEM_LINUX:
        {
            if (arguments.link_libcpp && !arguments.link_libc)
            {
                failed_execution();
            }

            if (arguments.link_libc)
            {
                auto crt_path = linux_crt_find_path();

                *vb_add(&args, 1) = "-dynamic-linker";

                String dynamic_linker_filename;
                switch (arguments.target.cpu)
                {
                case CPU_ARCH_X86_64:
                    dynamic_linker_filename = strlit("ld-linux-x86-64.so.2");
                    break;
                case CPU_ARCH_AARCH64:
                    dynamic_linker_filename = strlit("ld-linux-aarch64.so.1");
                    break;
                }

                linux_add_crt_item(arena, &args, crt_path, dynamic_linker_filename);
                linux_add_crt_item(arena, &args, crt_path, strlit("crt1.o"));

                *vb_add(&args, 1) = "-L";
                *vb_add(&args, 1) = string_to_c(crt_path);


                *vb_add(&args, 1) = "--as-needed";
                *vb_add(&args, 1) = "-lm";
                *vb_add(&args, 1) = "-lpthread";
                *vb_add(&args, 1) = "-lc";
                *vb_add(&args, 1) = "-ldl";
                *vb_add(&args, 1) = "-lrt";
                *vb_add(&args, 1) = "-lutil";

                linux_add_crt_item(arena, &args, crt_path, strlit("crtn.o"));

                if (arguments.link_libcpp)
                {
                    // TODO: implement better path finding
                    linux_add_crt_item(arena, &args, crt_path, strlit("libstdc++.so.6"));
                }
            }

            for (u64 i = 0; i < arguments.libraries.length; i += 1)
            {
                auto library = arguments.libraries.pointer[i];
                String library_pieces[] = {
                    strlit("-l"),
                    library,
                };
                auto library_argument = arena_join_string(arena, (Slice(String))array_to_slice(library_pieces));
                *vb_add(&args, 1) = string_to_c(library_argument);
            }
        } break;
    case OPERATING_SYSTEM_MAC:
        {
            *vb_add(&args, 1) = "-dynamic";
            *vb_add(&args, 1) = "-platform_version";
            *vb_add(&args, 1) = "macos";
            *vb_add(&args, 1) = "15.0.0";
            *vb_add(&args, 1) = "15.0.0";
            *vb_add(&args, 1) = "-arch";
            *vb_add(&args, 1) = "arm64";
            *vb_add(&args, 1) = "-syslibroot";
            *vb_add(&args, 1) = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk";

            if (string_ends_with(arguments.out_path, strlit(".dylib")))
            {
                *vb_add(&args, 1) = "-e";
                *vb_add(&args, 1) = "_main";
            }

            *vb_add(&args, 1) = "-lSystem";

            if (arguments.link_libcpp)
            {
                *vb_add(&args, 1) = "-lc++";
            }
        } break;
    case OPERATING_SYSTEM_WINDOWS:
        {
            if (arguments.link_libcpp && !arguments.link_libc)
            {
                failed_execution();
            }

            auto msvc_path = windows_msvc_find_path();

            if (arguments.link_libc)
            {
                *vb_add(&args, 1) = "-defaultlib:libcmt";

                {
                    // String parts[] = {
                    //     strlit("-libpath:"),
                    //     msvc_path,
                    //     strlit("/"),
                    //     strlit("VC/Tools/MSVC/14.41.34120/lib/x64"),
                    // };
                    // auto arg = arena_join_string(arena, (Slice(String)) array_to_slice(parts));
                }

                if (arguments.link_libcpp)
                {
                    todo();
                }

                for (u64 i = 0; i < arguments.objects.length; i += 1)
                {
                    *vb_add(&args, 1) = string_to_c(arguments.objects.pointer[i]);
                }
            }
            // clang -v main.c 
            // "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.41.34120\\bin\\Hostx64\\x64\\link.exe" -out:a.exe -defaultlib:libcmt -defaultlib:oldnames "-libpath:C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.41.34120\\lib\\x64" "-libpath:C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.41.34120\\atlmfc\\lib\\x64" "-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\ucrt\\x64" "-libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.22621.0\\um\\x64" "-libpath:C:\\Users\\David\\scoop\\apps\\llvm\\19.1.3\\lib\\clang\\19\\lib\\windows" -nologo "C:\\Users\\David\\AppData\\Local\\Temp\\main-706820.o"
        } break;
    }

    return (SliceP(char)){ .pointer = args.pointer, .length = args.length };
}
