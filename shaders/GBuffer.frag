#version 450 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gEffect;

layout (location = 0) in vec3 viewPos;
layout (location = 1) in vec2 TexCoords;
layout (location = 2) in vec3 normal;

layout (set = 1, binding = 0) uniform sampler2D texAlbedo;
layout (set = 1, binding = 1) uniform sampler2D texNormal;
layout (set = 1, binding = 2) uniform sampler2D texRoughness;
layout (set = 1, binding = 3) uniform sampler2D texMetalness;
layout (set = 1, binding = 4) uniform sampler2D texAO;

const float nearPlane = 1.0f;
const float farPlane = 1000.0f;

float LinearizeDepth(float depth);
vec3 computeTexNormal(vec3 viewNormal, vec3 texNormal);
void main()
{
    vec3 texNormal = normalize(texture(texNormal, TexCoords).rgb);
    texNormal.g = -texNormal.g;   // In case the normal map was made with DX3D coordinates system in mind

	gPosition = vec4(viewPos, LinearizeDepth(gl_FragCoord.z));

    gAlbedo.rgb = vec3(texture(texAlbedo, TexCoords));
    gAlbedo.a =  vec3(texture(texRoughness, TexCoords)).r;
    gNormal.rgb = computeTexNormal(normal, texNormal);
    gNormal.a = vec3(texture(texMetalness, TexCoords)).r;
    gEffect.r = vec3(texture(texAO, TexCoords)).r;
    //gEffect.gb = fragPosA - fragPosB;
}

float LinearizeDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}


vec3 computeTexNormal(vec3 viewNormal, vec3 texNormal)
{
    vec3 dPosX  = dFdx(viewPos);
    vec3 dPosY  = dFdy(viewPos);
    vec2 dTexX = dFdx(TexCoords);
    vec2 dTexY = dFdy(TexCoords);

    vec3 normal = normalize(viewNormal);
    vec3 tangent = normalize(dPosX * dTexY.t - dPosY * dTexX.t);
    vec3 binormal = -normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, binormal, normal);

    return normalize(TBN * texNormal);
}
