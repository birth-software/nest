#define unreachable() __builtin_unreachable()
#include <llvm-c/Core.h>
#include <std/os.h>
#include <bloat-buster/base.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include <llvm/MC/TargetRegistry.h>

#include <llvm/Support/TargetSelect.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm-c/TargetMachine.h>

#define string_ref(lit) StringRef(lit, strlit_len(lit))

namespace llvm
{
#define llvm_initialize_macro(target) \
    LLVMInitialize ## target ## Target();\
    LLVMInitialize ## target ## TargetInfo();\
    LLVMInitialize ## target ## TargetMC();\
    LLVMInitialize ## target ## AsmParser();\
    LLVMInitialize ## target ## AsmPrinter()

    fn void target_initialize(CpuArchitecture architecture)
    {
        // These are meant to be called globally, so if this code is ever threaded, we need to call this code only once
        switch (architecture)
        {
            case CPU_ARCH_X86_64:
                {
                    llvm_initialize_macro(X86);
                } break;
            case CPU_ARCH_AARCH64:
                {
                    llvm_initialize_macro(AArch64);
                } break;
        }
    }

    EXPORT void llvm_codegen(CodegenOptions options, String object_path)
    {
        target_initialize(options.target.cpu);

        auto context = LLVMContext();
        auto module = Module(string_ref("first"), context);
        std::string error_message;

        // TODO: debug builder
        // TODO: attributes

        {
            u32 return_bit_count = 32;
            auto* return_type = IntegerType::get(context, return_bit_count);
            ArrayRef<Type*> parameter_types = {};
            u8 is_var_args = 0;
            auto* function_type = FunctionType::get(return_type, parameter_types, is_var_args);
            auto function_name = string_ref("main");
            auto linkage = GlobalValue::LinkageTypes::ExternalLinkage;
            u32 address_space = 0;
            auto* function = Function::Create(function_type, linkage, address_space, function_name, &module);

            auto builder = IRBuilder<>(context);
            auto entry_block_name = string_ref("entry");
            auto* basic_block = BasicBlock::Create(context, entry_block_name, function, 0);
            builder.SetInsertPoint(basic_block);
            u64 return_value_int = 0;
            u8 is_signed = 0;
            auto* return_value = ConstantInt::get(context, APInt(return_bit_count, return_value_int, is_signed));
            builder.CreateRet(return_value);

            {
                raw_string_ostream message_stream(error_message);

                if (verifyModule(module, &message_stream))
                {
                    // Failure
                    auto& error_std_string = message_stream.str();
                    auto error_string = String{ .pointer = (u8*)error_std_string.data(), .length = error_std_string.length() };
                    print("Verification for module failed:\n{s}\n", error_string);
                    failed_execution();
                }
            }
        }
        
        // TODO: make a more correct logic
        StringRef target_triple;
        switch (options.target.os)
        {
            case OPERATING_SYSTEM_LINUX:
                target_triple = string_ref("x86_64-unknown-linux-gnu");
                break;
            case OPERATING_SYSTEM_MAC:
                target_triple = string_ref("aarch64-apple-macosx-none");
                break;
            case OPERATING_SYSTEM_WINDOWS:
                target_triple = string_ref("x86_64-windows-gnu");
                break;
        }

        const Target* target = TargetRegistry::lookupTarget(target_triple, error_message);
        if (!target)
        {
            String string = { .pointer = (u8*)error_message.data(), .length = error_message.length() };
            print("Could not find target: {s}\n", string);
            failed_execution();
        }

        module.setTargetTriple(target_triple);

        // TODO:
        auto cpu_model = string_ref("");
        auto cpu_features = string_ref("");

        TargetOptions target_options;
        std::optional<Reloc::Model> relocation_model = std::nullopt;
        std::optional<CodeModel::Model> code_model = std::nullopt;
        auto codegen_optimization_level = CodeGenOptLevel::None;
        u8 jit = 0;

        auto* target_machine = target->createTargetMachine(target_triple, cpu_model, cpu_features, target_options, relocation_model, code_model, codegen_optimization_level, jit);
        auto data_layout = target_machine->createDataLayout();
        module.setDataLayout(data_layout);

        // TODO: optimizations

        SmallString<0> object_string;
        raw_svector_ostream object_stream(object_string);
        auto file_type = CodeGenFileType::ObjectFile;
        legacy::PassManager pass;

        assert(target_machine->isCompatibleDataLayout(module.getDataLayout()));
        raw_pwrite_stream* dwo_stream = 0;
        if (target_machine->addPassesToEmitFile(pass, object_stream, dwo_stream, file_type)) {
            failed_execution();
        }

        pass.run(module);

        assert(object_path.pointer);
        assert(object_path.length);

        file_write(FileWriteOptions{
            .path = object_path,
            .content = { .pointer = (u8*)object_string.str().data(), .length = object_string.str().size() },
            .executable = 1,
        });
    }
}
