#version 450 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gEffect;

layout (location = 0) in vec3 viewPos;
layout (location = 1) in vec2 TexCoords;
layout (location = 2) in vec3 normal;

const float nearPlane = 1.0f;
const float farPlane = 1000.0f;

float LinearizeDepth(float depth);
void main()
{
	gPosition = vec4(viewPos, LinearizeDepth(gl_FragCoord.z));
    gAlbedo = vec4(TexCoords, 0.0, 1.0);
    gNormal = vec4(normal, 1.0);
    gEffect = vec4(1.0, 0.0, 0.0, 0.0);
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
