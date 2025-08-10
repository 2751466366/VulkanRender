#pragma once
#include "EasyVKStart.h"
#include "common.h"

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::mat4 model = glm::mat4(1.f);
    vertexBuffer vertexBuffer;
    indexBuffer indexBuffer;
    Mesh() = default;
    Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    ~Mesh() = default;
    void LoadCube()
    {
        vertices = {
            // 前面 (+Z)
            { { -0.5f, -0.5f,  0.5f }, { 0, 0, 1 }, {0, 0} },
            { {  0.5f, -0.5f,  0.5f }, { 0, 0, 1 }, {1, 0} },
            { {  0.5f,  0.5f,  0.5f }, { 0, 0, 1 }, {1, 1} },
            { { -0.5f,  0.5f,  0.5f }, { 0, 0, 1 }, {0, 1} },
            // 后面 (-Z)
            { {  0.5f, -0.5f, -0.5f }, { 0, 0, -1 }, {0, 0} },
            { { -0.5f, -0.5f, -0.5f }, { 0, 0, -1 }, {1, 0} },
            { { -0.5f,  0.5f, -0.5f }, { 0, 0, -1 }, {1, 1} },
            { {  0.5f,  0.5f, -0.5f }, { 0, 0, -1 }, {0, 1} },
            // 左面 (-X)
            { { -0.5f, -0.5f, -0.5f }, { -1, 0, 0 }, {0, 0} },
            { { -0.5f, -0.5f,  0.5f }, { -1, 0, 0 }, {1, 0} },
            { { -0.5f,  0.5f,  0.5f }, { -1, 0, 0 }, {1, 1} },
            { { -0.5f,  0.5f, -0.5f }, { -1, 0, 0 }, {0, 1} },
            // 右面 (+X)
            { {  0.5f, -0.5f,  0.5f }, { 1, 0, 0 }, {0, 0} },
            { {  0.5f, -0.5f, -0.5f }, { 1, 0, 0 }, {1, 0} },
            { {  0.5f,  0.5f, -0.5f }, { 1, 0, 0 }, {1, 1} },
            { {  0.5f,  0.5f,  0.5f }, { 1, 0, 0 }, {0, 1} },
            // 上面 (+Y)
            { { -0.5f,  0.5f,  0.5f }, { 0, 1, 0 }, {0, 0} },
            { {  0.5f,  0.5f,  0.5f }, { 0, 1, 0 }, {1, 0} },
            { {  0.5f,  0.5f, -0.5f }, { 0, 1, 0 }, {1, 1} },
            { { -0.5f,  0.5f, -0.5f }, { 0, 1, 0 }, {0, 1} },
            // 下面 (-Y)
            { { -0.5f, -0.5f, -0.5f }, { 0, -1, 0 }, {0, 0} },
            { {  0.5f, -0.5f, -0.5f }, { 0, -1, 0 }, {1, 0} },
            { {  0.5f, -0.5f,  0.5f }, { 0, -1, 0 }, {1, 1} },
            { { -0.5f, -0.5f,  0.5f }, { 0, -1, 0 }, {0, 1} },
        };
        indices = {
            // 前面
            0, 1, 2, 2, 3, 0,
            // 后面
            4, 5, 6, 6, 7, 4,
            // 左面
            8, 9,10,10,11, 8,
            // 右面
            12,13,14,14,15,12,
            // 上面
            16,17,18,18,19,16,
            // 下面
            20,21,22,22,23,20
        };
        vertexBuffer.Create(vertices.size() * sizeof(Vertex));
        vertexBuffer.TransferData(vertices.data(), vertices.size() * sizeof(Vertex));
        indexBuffer.Create(indices.size() * sizeof(uint32_t));
        indexBuffer.TransferData(indices.data(), indices.size() * sizeof(uint32_t));
    }

    void SetWorldPos(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f)) {
        model = glm::translate(model, pos);
    }

    void Draw(const commandBuffer& commandBuffer)
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer.Address(), &offset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
    }
};