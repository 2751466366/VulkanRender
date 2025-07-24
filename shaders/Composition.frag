#version 450 core

layout(location = 0) in vec2 i_Position;
layout(location = 0) out vec4 o_Color;

layout(binding = 0, input_attachment_index = 0) uniform subpassInput u_GBuffers[3];
uint mode = 0; // 0, 1, 2,

void main() {
	o_Color = subpassLoad(u_GBuffers[1]);
	o_Color.w = 1.0;
}