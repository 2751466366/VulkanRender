#version 450 core

layout(location = 0) in vec2 TexCoords;
layout(location = 1) in vec3 envMapCoords;
layout(location = 0) out vec4 o_Color;

layout(binding = 0, input_attachment_index = 0) uniform subpassInput u_GBuffers[4];
layout (binding = 0) uniform sampler2D gPosition;
layout (binding = 1) uniform sampler2D gAlbedo;
layout (binding = 2) uniform sampler2D gNormal;
layout (binding = 3) uniform sampler2D gEffect;

layout(binding = 4) uniform samplerCube envMap;
layout(binding = 5) uniform samplerCube envMapIrradiance;
layout(binding = 6) uniform samplerCube envMapPrefilter;
layout(binding = 7) uniform sampler2D envMapLUT;

layout(binding = 9) uniform transformData {
    mat4 view;
} transform;

// Light source(s) informations
struct LightObject
{
    vec3 position;
    vec3 direction;
    vec4 color;
    float radius;
};
layout(binding = 10) uniform LightInfo {
    uint pointLightNum;
    uint dirLightNum;
    LightObject pointLights[10];
    LightObject dirLights[10];
} lightInfo;

uint pointMode = 1;
uint directionalMode = 1;
uint iblMode = 1;

const float EPS = 1e-5;
const float PI = 3.14159265359f;
const float prefilterLODLevel = 4.0f;
vec3 materialF0 = vec3(0.04f);

vec3 colorLinear(vec3 colorVector);
float saturate(float f);
vec2 getSphericalCoord(vec3 normalCoord);
float Fd90(float NoL, float roughness);
float KDisneyTerm(float NoL, float NoV, float roughness);
vec3 computeFresnelSchlick(float NdotV, vec3 F0);
vec3 computeFresnelSchlickRoughness(float NdotV, vec3 F0, float roughness);
float computeDistributionGGX(vec3 N, vec3 H, float roughness);
float computeGeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness);

vec3 colorSRGB(vec3 colorVector);

void main() {
    vec3 envColor = texture(envMap, normalize(envMapCoords)).rgb;

    vec3 viewPos = textureLod(gPosition, TexCoords, 0).rgb;
    float depth = textureLod(gPosition, TexCoords, 0).a;
    vec3 albedo = colorLinear(textureLod(gAlbedo, TexCoords, 0).rgb);
    float roughness = textureLod(gAlbedo, TexCoords, 0).a;
    vec3 normal = textureLod(gNormal, TexCoords, 0).rgb;
    float metalness = textureLod(gNormal, TexCoords, 0).a;
    float ao = textureLod(gEffect, TexCoords, 0).r;
    //vec2 velocity = textureLod(gEffect, TexCoords, 0).gb;

    vec3 color = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);

    if(abs(depth - 1.0) < EPS) {
        color = envColor;
    } else {
        vec3 V = normalize(- viewPos);
        vec3 N = normalize(normal);
        vec3 R = reflect(-V, N);

        float NdotV = max(dot(N, V), 0.0001f);

        // Fresnel (Schlick) computation (F term)
        vec3 F0 = mix(materialF0, albedo, metalness);
        vec3 F = computeFresnelSchlick(NdotV, F0);

        // Energy conservation
        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= (1.0f - metalness);

        if (pointMode == 1) {
            for (int i = 0; i < lightInfo.pointLightNum; i++) {
                vec3 L = normalize(lightInfo.pointLights[i].position - viewPos);
                vec3 H = normalize(L + V);

                vec3 lightColor = colorLinear(lightInfo.pointLights[i].color.rgb);
                float distanceL = length(lightInfo.pointLights[i].position - viewPos);
                float attenuation = 1.0f / (distanceL * distanceL); // Quadratic attenuation
                //float attenuation =
                //    pow(saturate(1 - pow(distanceL / lightInfo.pointLights[i].radius, 4)), 2) /
                //        (distanceL * distanceL + 1); // UE4 attenuation

                // Light source dependent BRDF term(s)
                float NdotL = saturate(dot(N, L));
                // Radiance computation
                vec3 kRadiance = lightColor * attenuation;
                // Diffuse component computation
                diffuse = albedo / PI;
                // Disney diffuse term
                float kDisney = KDisneyTerm(NdotL, NdotV, roughness);
                // Distribution (GGX) computation (D term)
                float D = computeDistributionGGX(N, H, roughness);
                // Geometry attenuation (GGX-Smith) computation (G term)
                float G = computeGeometryAttenuationGGXSmith(NdotL, NdotV, roughness);
                // Specular component computation
                specular = (F * D * G) / (4.0f * NdotL * NdotV + 0.0001f);

                color += (diffuse * kD + specular) * kRadiance * NdotL;
            }
        }
        if (directionalMode == 1) {
            for (int i = 0; i < lightInfo.dirLightNum; i++) {
                vec3 L = normalize(- lightInfo.dirLights[i].direction);
                vec3 H = normalize(L + V);

                vec3 lightColor = colorLinear(lightInfo.dirLights[i].color.rgb);

                // Light source dependent BRDF term(s)
                float NdotL = saturate(dot(N, L));
                // Diffuse component computation
                diffuse = albedo / PI;
                // Disney diffuse term
                float kDisney = KDisneyTerm(NdotL, NdotV, roughness);
                // Distribution (GGX) computation (D term)
                float D = computeDistributionGGX(N, H, roughness);
                // Geometry attenuation (GGX-Smith) computation (G term)
                float G = computeGeometryAttenuationGGXSmith(NdotL, NdotV, roughness);
                // Specular component computation
                specular = (F * D * G) / (4.0f * NdotL * NdotV + 0.0001f);

                color += (diffuse * kD + specular) * lightColor * NdotL;
            }
        }
        if (iblMode == 1) {
	        F = computeFresnelSchlickRoughness(NdotV, F0, roughness);
            kS = F;
            kD = vec3(1.0f) - kS;
            kD *= (1.0f - metalness);

            // Diffuse irradiance computation
            vec3 diffuseIrradiance = texture(envMapIrradiance, N * mat3(transform.view)).rgb;
            diffuseIrradiance *= albedo;

            // Specular radiance computation
            vec3 specularRadiance = textureLod(envMapPrefilter, R * mat3(transform.view), roughness * prefilterLODLevel).rgb;
            vec2 brdfSampling = texture(envMapLUT, vec2(NdotV, roughness)).rg;
            specularRadiance *= (F * brdfSampling.x + brdfSampling.y);

            vec3 ambientIBL = (diffuseIrradiance * kD) + specularRadiance;

            color += ambientIBL;
        }

        color *= ao;
	}
    color = colorSRGB(color);
    o_Color = vec4(color, 1.0);
}

vec3 colorLinear(vec3 colorVector)
{
    vec3 linearColor = pow(colorVector.rgb, vec3(2.2f));

    return linearColor;
}


float saturate(float f)
{
    return clamp(f, 0.0f, 1.0f);
}


vec2 getSphericalCoord(vec3 normalCoord)
{
    float phi = acos(-normalCoord.y);
    float theta = atan(1.0f * normalCoord.x, -normalCoord.z) + PI;

    return vec2(theta / (2.0f * PI), phi / PI);
}


float Fd90(float NoL, float roughness)
{
    return (2.0f * NoL * roughness) + 0.4f;
}


float KDisneyTerm(float NoL, float NoV, float roughness)
{
    return (1.0f + Fd90(NoL, roughness) * pow(1.0f - NoL, 5.0f)) * (1.0f + Fd90(NoV, roughness) * pow(1.0f - NoV, 5.0f));
}


vec3 computeFresnelSchlick(float NdotV, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - NdotV, 5.0f);
}


vec3 computeFresnelSchlickRoughness(float NdotV, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - NdotV, 5.0f);
}


float computeDistributionGGX(vec3 N, vec3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    return (alpha2) / (PI * (NdotH2 * (alpha2 - 1.0f) + 1.0f) * (NdotH2 * (alpha2 - 1.0f) + 1.0f));
}


float computeGeometryAttenuationGGXSmith(float NdotL, float NdotV, float roughness)
{
    float NdotL2 = NdotL * NdotL;
    float NdotV2 = NdotV * NdotV;
    float kRough2 = roughness * roughness + 0.0001f;

    float ggxL = (2.0f * NdotL) / (NdotL + sqrt(NdotL2 + kRough2 * (1.0f - NdotL2)));
    float ggxV = (2.0f * NdotV) / (NdotV + sqrt(NdotV2 + kRough2 * (1.0f - NdotV2)));

    return ggxL * ggxV;
}

vec3 colorSRGB(vec3 colorVector)
{
  vec3 srgbColor = pow(colorVector.rgb, vec3(1.0f / 2.2f));

  return srgbColor;
}