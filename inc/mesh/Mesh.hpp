#pragma once
#include "EasyVKStart.h"
#include "common.h"
#include "BVH.hpp"

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::mat4 model = glm::mat4(1.f);
    Mesh() = default;
    Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
    {
        this->vertices = vertices;
        this->indices = indices;
    }
    Mesh(const Mesh& mesh)
    {
        this->vertices = mesh.vertices;
        this->indices = mesh.indices;
        this->model = mesh.model;
    }
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
    }

    void LoadQuad()
    {
        vertices = {
            { {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
            { {-1.0f,-1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
            { { 1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
            { { 1.0f,-1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
        };
        indices = {
            0, 1, 2, 3
        };
    }

    void SetWorldPos(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f)) {
        model = glm::translate(model, pos);
    }

    void BuildBVH()
    {
        bvh.Init(vertices, indices);
    }

    void PrepareMeshData()
    {
        if (isInit) {
            std::cout << "mesh already init\n";
            return;
        }
        isInit = true;
        vertexBuffer.Create(vertices.size() * sizeof(Vertex));
        vertexBuffer.TransferData(vertices.data(), vertices.size() * sizeof(Vertex));
        indexBuffer.Create(indices.size() * sizeof(uint32_t));
        indexBuffer.TransferData(indices.data(), indices.size() * sizeof(uint32_t));
    }

    void PrepareVBHData(uint32_t level, bool needRecreate)
    {
        const std::vector<std::array<glm::vec3, 2>>& boxList = bvh.GetBoxList();
        std::vector<int> nodes = bvh.GetNodesAtLevel(level);
        std::cout << "nodes: [ ";
        for (int i = 0; i < nodes.size(); i++) {
            std::cout << nodes[i] << ", ";
        }
        std::cout << "]\n";
        bvhVertices.resize(nodes.size() * 8);
        bvhIndices.resize(nodes.size() * 24);
        for (int n = 0; n < nodes.size(); n++) {
            const std::array<glm::vec3, 2>& box = boxList[nodes[n]];
            const glm::vec3 vmin  = box[0];
            const glm::vec3 vmax  = box[1];
            const glm::vec3 vmid  = (vmax + vmin) * 0.5f;
            glm::vec3 color       = CoordToColor(vmid);
            int vertexOffset      = 8 * n;
            int indexOffset       = 24 * n;

            glm::vec3 aabb[8] = {
                {vmin.x, vmin.y, vmin.z},  // 0
                {vmax.x, vmin.y, vmin.z},  // 1
                {vmin.x, vmax.y, vmin.z},  // 2
                {vmax.x, vmax.y, vmin.z},  // 3
                {vmin.x, vmin.y, vmax.z},  // 4
                {vmax.x, vmin.y, vmax.z},  // 5
                {vmin.x, vmax.y, vmax.z},  // 6
                {vmax.x, vmax.y, vmax.z}   // 7
            };
            uint32_t faces[24] = {
                0,1,  1,3,  3,2,  2,0,
                4,5,  5,7,  7,6,  6,4,
                0,4,  1,5,  2,6,  3,7
            };
            for (int v = 0; v < 8; v++) {
                bvhVertices[vertexOffset + v] = {
                    .position = aabb[v],
                    .color = color
                };
            }
            for (int i = 0; i < 24; i++) {
                bvhIndices[indexOffset + i] = vertexOffset + faces[i];
            }
        }
        if (!needRecreate) {
            bvhVertexBuffer.Create(bvhVertices.size() * sizeof(PosColor));
            bvhIndexuffer.Create(bvhIndices.size() * sizeof(uint32_t));
        } else {
            bvhVertexBuffer.Recreate(bvhVertices.size() * sizeof(PosColor));
            bvhIndexuffer.Recreate(bvhIndices.size() * sizeof(uint32_t));
        }
        bvhVertexBuffer.TransferData(bvhVertices.data(), bvhVertices.size() * sizeof(PosColor));
        bvhIndexuffer.TransferData(bvhIndices.data(), bvhIndices.size() * sizeof(uint32_t));
    }

    void Draw(const commandBuffer& commandBuffer)
    {
        if (!isInit) {
            PrepareMeshData();
        }
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer.Address(), &offset);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indices.size(), 1, 0, 0, 0);
    }

    void DrawBVH(const commandBuffer& commandBuffer, int level)
    {
        if (!bvh.IsInited()) {
            return;
        }
        bool needRecreate = (levelOfData != -1);
        level = std::min(level, bvh.GetMaxLevel());
        if (level != levelOfData) {
            PrepareVBHData(level, needRecreate);
            levelOfData = level;
        }
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, bvhVertexBuffer.Address(), &offset);
        vkCmdBindIndexBuffer(commandBuffer, bvhIndexuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, bvhIndices.size(), 1, 0, 0, 0);
    }

private:
    bool isInit = false;
    vulkan::vertexBuffer vertexBuffer;
    vulkan::indexBuffer indexBuffer;

    BVH bvh;
    int levelOfData = -1;
    std::vector<PosColor> bvhVertices;
    std::vector<uint32_t> bvhIndices;
    vulkan::vertexBuffer bvhVertexBuffer;
    vulkan::indexBuffer bvhIndexuffer;
};