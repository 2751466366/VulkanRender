#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 0) out vec2 TexCoords;
layout(location = 1) out vec3 envMapCoords;
layout(binding = 8) uniform transformData {
    mat4 invView;
    mat4 invProj;
} invTransform;

void main() {
	TexCoords = texCoords;
    vec4 unprojCoords = (invTransform.invProj * vec4(position.xy, vec2(1.0f)));
    envMapCoords = (invTransform.invView * unprojCoords).xyz;

    gl_Position = vec4(position, 1.0f);
}