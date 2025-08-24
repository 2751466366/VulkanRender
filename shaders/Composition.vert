#version 450 core

vec2 positions[6] = {
	{-1,-1},
	{ 1,-1},
	{ 1, 1},
	{ 1, 1},
	{-1, 1},
	{-1,-1}
};

layout(location = 0) out vec3 i_Position;
layout(set = 0, binding = 2) uniform transformData {
    mat4 invView;
    mat4 invProj;
} invTransform;

void main() {
	vec2 ndc = positions[gl_VertexIndex];

	vec4 clip = vec4(ndc.x, ndc.y, 1.0, 1.0);
    vec4 view = invTransform.invProj * clip;
    vec3 worldPos = (invTransform.invView * view).xyz;

	i_Position = worldPos;
	gl_Position = vec4(ndc, 0, 1);
}