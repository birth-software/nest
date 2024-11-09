#include <bloat-buster/shader_compilation.h>
#include <glslang/Include/glslang_c_interface.h>


// Required for use of glslang_default_resource
#include <glslang/Public/resource_limits_c.h>

typedef struct SpirVBinary {
    uint32_t *words; // SPIR-V words
    int size; // number of words in SPIR-V binary
} SpirVBinary;

fn SpirVBinary compileShaderToSPIRV_Vulkan(Arena* arena, glslang_stage_t stage, String shader_source, String filename)
{
    if (!glslang_initialize_process())
    {
        failed_execution();
    }

    const glslang_input_t input = {
        .language = GLSLANG_SOURCE_GLSL,
        .stage = stage,
        .client = GLSLANG_CLIENT_VULKAN,
        .client_version = GLSLANG_TARGET_VULKAN_1_3,
        .target_language = GLSLANG_TARGET_SPV,
        .target_language_version = GLSLANG_TARGET_SPV_1_6,
        .code = string_to_c(shader_source),
        .default_version = 450,
        .default_profile = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible = false,
        .messages = GLSLANG_MSG_DEFAULT_BIT,
        .resource = glslang_default_resource(),
    };

    glslang_shader_t* shader = glslang_shader_create(&input);


    SpirVBinary bin = {
        .words = NULL,
        .size = 0,
    };
    if (!glslang_shader_preprocess(shader, &input))	{
        print("GLSL preprocessing failed {s}\n", filename);
        print("{cstr}\n", glslang_shader_get_info_log(shader));
        print("{cstr}\n", glslang_shader_get_info_debug_log(shader));
        print("{cstr}\n", input.code);
        glslang_shader_delete(shader);
        return bin;
    }

    if (!glslang_shader_parse(shader, &input)) {
        print("GLSL parsing failed {s}\n", filename);
        print("{cstr}\n", glslang_shader_get_info_log(shader));
        print("{cstr}\n", glslang_shader_get_info_debug_log(shader));
        print("{cstr}\n", glslang_shader_get_preprocessed_code(shader));
        glslang_shader_delete(shader);
        return bin;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
        print("GLSL linking failed {s}\n", filename);
        print("{cstr}\n", glslang_program_get_info_log(program));
        print("{cstr}\n", glslang_program_get_info_debug_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        return bin;
    }

    glslang_program_SPIRV_generate(program, stage);

    bin.size = glslang_program_SPIRV_get_size(program);
    bin.words = arena_allocate(arena, u32, bin.size);
    glslang_program_SPIRV_get(program, bin.words);

    const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages)
        print("({s}) {cstr}\b", filename, spirv_messages);

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return bin;
}

String compile_shader(Arena* arena, String path, ShaderStage shader_stage)
{
    auto file = file_read(arena, path);

    glslang_stage_t stage;
    switch (shader_stage)
    {
        case SHADER_STAGE_VERTEX:
            stage = GLSLANG_STAGE_VERTEX;
            break;
        case SHADER_STAGE_FRAGMENT:
            stage = GLSLANG_STAGE_FRAGMENT;
            break;
    }

    auto result = compileShaderToSPIRV_Vulkan(arena, stage, file, path);

    return (String) {
        .pointer = (u8*)result.words,
        .length = result.size * sizeof(*result.words),
    };
}
