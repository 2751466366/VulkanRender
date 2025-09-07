#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout (location = 0) out vec3 viewPos;
layout (location = 1) out vec2 TexCoords;
layout (location = 2) out vec3 worldNormal;

layout(set = 0, binding = 0) uniform transformData {
    mat4 view;
    mat4 proj;
} transform;

layout(set = 1, binding = 5) uniform modelTransformData {
    mat4 model;
} modelTransform;

void main()
{
    // View Space
    vec4 viewFragPos = transform.view * modelTransform.model * vec4(position, 1.0f);
    viewPos = viewFragPos.xyz;

    TexCoords = texCoords;

    mat3 normalMatrix = transpose(inverse(mat3(transform.view * modelTransform.model)));
    worldNormal = normalMatrix * normal;

    gl_Position = transform.proj * viewFragPos;
}