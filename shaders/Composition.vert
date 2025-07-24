#version 450 core

vec2 positions[4] = {
	{-1,-1},
	{-1, 1},
	{ 1,-1},
	{ 1, 1}
};

layout(location = 0) out vec2 o_Position;

void main() {
	o_Position = positions[gl_VertexIndex];
	gl_Position = vec4(o_Position, 0, 1);
}