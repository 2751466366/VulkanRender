#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <span>
#include <memory>
#include <functional>
#include <concepts>
#include <format>
#include <chrono>
#include <numeric>
#include <numbers>
#include <algorithm>
#include <list>
#include <set>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct PosColor {
    glm::vec3 position;
    glm::vec3 color;
};

// 伪随机 3D → 1D 杂凑
inline float Hash3(glm::vec3 p) {
    // 经典质数乘法＋黄金角
    uint32_t h = uint32_t(p.x * 73856093) ^
        uint32_t(p.y * 19349663) ^
        uint32_t(p.z * 83492791);
    h = h * 0x9e3779b9;               // 黄金比例
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    return float(h) * 2.3283064365386963e-10f; // 2^-32
}

// 坐标 → RGB
inline glm::vec3 CoordToColor(glm::vec3 pos) {
    return glm::vec3(
        Hash3(pos + glm::vec3(0.0f, 0.0f, 0.0f)),
        Hash3(pos + glm::vec3(1.618f, 2.718f, 3.142f)),
        Hash3(pos + glm::vec3(5.0f, 7.0f, 11.0f))
    );
}

#define GET_ARRAY_NUM(arr) (sizeof(arr) / sizeof(arr[0]))