#include <std/base.h>
#include <std/os.h>

typedef enum ShaderStage : u8
{
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_FRAGMENT,
} ShaderStage;

EXPORT String compile_shader(Arena* arena, String path, ShaderStage shader_stage);
