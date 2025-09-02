#pragma once
#include "Mesh.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class Model {
public:
    void LoadModel(const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }

        this->directory = path.substr(0, path.find_last_of('/'));
        this->ProcessNode(scene->mRootNode, scene);
    }

    void Draw(const commandBuffer& commandBuffer)
    {
        for (uint32_t i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw(commandBuffer);
    }

    void SetModel(glm::mat4 model) { this->model = model; }
    const glm::mat4 &GetModel() { return model; }

    void SetWorldPos(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f)) {
        model = glm::translate(model, pos);
    }

    void LoadTexuture(std::string path)
    {
        std::string name = path.substr(path.find_last_of('/'));
        std::vector<std::string> pathList;
        pathList.push_back(path + name + "_albedo.png");
        pathList.push_back(path + name + "_normal.png");
        pathList.push_back(path + name + "_roughness.png");
        pathList.push_back(path + name + "_metalness.png");
        pathList.push_back(path + name + "_ao.png");

        textureList.resize(5);
        samplerList.resize(5);
        textureInfoList.resize(5);
        for (int i = 0; i < 5; i++) {
            textureList[i].Create(
                pathList[i].c_str(),
                VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT
            );
            VkSamplerCreateInfo samplerCreateInfo = textureList[i].SamplerCreateInfo();
            samplerList[i].Create(samplerCreateInfo);
            textureInfoList[i] = {
                samplerList[i], textureList[i].ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
        }
    }

    const std::vector<VkDescriptorImageInfo>& GetTextureInfo()
    {
        return textureInfoList;
    }
private:
    std::vector<Mesh> meshes;
    std::vector<texture2d> textureList;
    std::vector<sampler> samplerList;
    std::vector<VkDescriptorImageInfo> textureInfoList;
    std::string directory;
    glm::mat4 model = glm::mat4(1.f);


    void ProcessNode(aiNode* node, const aiScene* scene)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes.push_back(this->ProcessMesh(mesh, scene));
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            this->ProcessNode(node->mChildren[i], scene);
        }

        /*uint32_t totalMeshSize = 0;
        std::vector<aiNode*> nodeList;
        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            totalMeshSize += node->mChildren[i].
        }*/
    }


    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;

            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoords = vec;
            }
            else
                vertex.texCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices);
    }
};