#pragma once

#include <std/base.h>
#include <std/os.h>

#include <std/render.h>

EXPORT String compile_shader(Arena* arena, String path, ShaderStage shader_stage);
