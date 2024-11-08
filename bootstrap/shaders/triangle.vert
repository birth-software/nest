#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_debug_printf : require

layout (location = 0) out vec4 outColor;

struct Vertex {
	vec4 position;
	vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	//load vertex data from device address
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = v.position;
	outColor = v.color;
    //debugPrintfEXT("Position: (%f, %f, %f)\n", v.position.x, v.position.y, v.position.z);
    //debugPrintfEXT("Color: (%f, %f, %f)\n", v.color.x, v.color.y, v.color.z);
}
