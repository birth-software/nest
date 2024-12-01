#version 450
#extension GL_EXT_nonuniform_qualifier : require


//shader input
layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in flat uint texture_index;

//output write
layout (location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 0) uniform sampler2D textures[];

void main() 
{
    vec4 sampled = texture(textures[texture_index], in_uv);
    out_frag_color = in_color * sampled;
}
