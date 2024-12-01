#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_debug_printf : require

//shader input
layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_screen_uv;
layout (location = 2) in flat uint texture_index;

//output write
layout (location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 0) uniform sampler2D textures[];

void main() 
{
    vec2 texture_size = textureSize(textures[texture_index], 0);
    vec2 uv = vec2(in_screen_uv.x / texture_size.x, in_screen_uv.y / texture_size.y);
    vec4 sampled = texture(textures[texture_index], uv);
    // debugPrintfEXT("In color: (%f, %f, %f, %f). Sampled: (%f, %f, %f, %f)\n", in_color.x, in_color.y, in_color.z, in_color.w, sampled.x, sampled.y, sampled.z, sampled.w);
    out_frag_color = in_color * sampled;
}
