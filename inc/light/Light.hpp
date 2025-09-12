#pragma once
#include "common.h"

// uniform std140 align
struct LightObject
{
    glm::vec3 position;
    uint32_t pad0;
    glm::vec3 direction;
    uint32_t pad1;
    glm::vec4 color;
    float radius;
    uint32_t pad2;
    uint32_t pad3;
    uint32_t pad4;
};