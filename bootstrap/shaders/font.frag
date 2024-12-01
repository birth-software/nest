#version 450

//shader input
layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec4 in_color;

//output write
layout (location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 0) uniform sampler2D atlas_texture;

void main() 
{
    vec4 sampled = texture(atlas_texture, in_uv);
    out_frag_color = in_color * sampled;
}
