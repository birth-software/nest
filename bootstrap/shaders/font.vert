#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_debug_printf : require

layout (location = 0) out vec2 out_uv;
layout (location = 1) out vec4 out_color;

struct Vertex {
    float x;
    float y;
    float uv_x;
    float uv_y;
    vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
   Vertex vertices[];
};

//push constants block
layout(push_constant) uniform constants
{
    VertexBuffer vertex_buffer;
    float width;
    float height;
} PushConstants;

void main() 
{
    //load vertex data from device address
    Vertex v = PushConstants.vertex_buffer.vertices[gl_VertexIndex];
    float width = PushConstants.width;
    float height = PushConstants.height;

    //output data
    gl_Position = vec4(2 * v.x / width - 1, 1 - (2 * v.y) / height, 0.0, 1.0);
    out_uv = vec2(v.uv_x, v.uv_y);
    out_color = v.color;
    //debugPrintfEXT("Position: (%f, %f, %f)\n", v.position.x, v.position.y, v.position.z);
    //debugPrintfEXT("Color: (%f, %f, %f)\n", v.color.x, v.color.y, v.color.z);
}
