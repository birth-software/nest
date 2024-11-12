#version 450

//shader input
layout (location = 0) in vec2 in_uv;

//output write
layout (location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 0) uniform sampler2D display_texture;

void main() 
{
    out_frag_color = texture(display_texture, in_uv);
}
