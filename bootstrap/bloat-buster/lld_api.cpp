#include <bloat-buster/lld_api.h>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/raw_ostream.h>
#include <lld/Common/Driver.h>
#include <std/os.h>

#define lld_api_function_signature(name) bool name(llvm::ArrayRef<const char *> args, llvm::raw_ostream &stdoutOS, llvm::raw_ostream &stderrOS, bool exitEarly, bool disableOutput)

#define lld_link_decl(link_name) \
namespace link_name \
{\
    lld_api_function_signature(link);\
}

typedef lld_api_function_signature(LinkerFunction);

namespace lld
{
    lld_link_decl(coff);
    lld_link_decl(elf);
    lld_link_decl(mingw);
    lld_link_decl(macho);
    lld_link_decl(wasm);
}

fn u8 lld_api_generic(LLDArguments args, LinkerFunction linker_function)
{
    auto arguments = llvm::ArrayRef(args.argument_pointer, args.argument_count);
    std::string stdout_string;
    llvm::raw_string_ostream stdout_stream(stdout_string);

    std::string stderr_string;
    llvm::raw_string_ostream stderr_stream(stderr_string);
    u8 result = linker_function(arguments, stdout_stream, stderr_stream, args.exit_early, args.disable_output);
    // assert(result == (stdout_string.length() == 0));
    // assert(result == (stderr_string.length() == 0));

    print_string(String{(u8*)stdout_string.data(), stdout_string.length()});
    print_string(String{(u8*)stderr_string.data(), stderr_string.length()});

    return result;
}

#define lld_api_function_impl(link_name) \
lld_api_function_decl(link_name)\
{\
    return lld_api_generic(args, lld::link_name::link);\
}

lld_api_function_impl(coff)
lld_api_function_impl(elf)
lld_api_function_impl(mingw)
lld_api_function_impl(macho)
lld_api_function_impl(wasm)
