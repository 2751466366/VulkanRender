#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

layout(location = 0) out vec3 cubeCoords;

layout(set = 0, binding = 0) uniform transformData {
    mat4 view;
    mat4 proj;
} transform;


void main()
{
    cubeCoords = position;
    vec4 clipPosition = transform.proj * mat4(mat3(transform.view)) * vec4(cubeCoords, 1.0f);

    gl_Position = clipPosition.xyww;
}