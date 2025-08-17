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

    gl_Position =  transform.proj * transform.view * vec4(position, 1.0);
}