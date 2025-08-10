#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec3 worldPos;

layout(set = 0, binding = 1) uniform sampler2D envMap;

float PI  = 3.14159265359f;

vec2 getSphericalCoord(vec3 normalCoord)
{
    float phi = acos(-normalCoord.y);
    float theta = atan(1.0f * normalCoord.x, -normalCoord.z) + PI;

    return vec2(theta / (2.0f * PI), phi / PI);
}


void main()
{		
    vec2 uv = getSphericalCoord(normalize(worldPos));
    vec3 color = texture(envMap, uv).rgb;
    
    fragColor = vec4(color, 1.0);
}