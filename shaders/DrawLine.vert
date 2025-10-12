#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout (location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform transformData {
    mat4 view;
    mat4 proj;
} transform;

layout(set = 1, binding = 0) uniform modelTransformData {
    mat4 model;
} modelTransform;

void main()
{
    gl_Position = transform.proj * transform.view * modelTransform.model * vec4(position, 1.0f);
    fragColor = color;
}