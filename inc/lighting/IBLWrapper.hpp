#pragma once
#include "VkBase+.h"
#include "common.h"
#include "CubeRenderPass.hpp"
#include "Mesh.hpp"

class IBLWrapper {
public:
    VkExtent2D windowSize = { 512, 512 };
	texture2d hdrTexture;
    sampler hdrTexSampler;
    Mesh cube;
    glm::mat4 envMapProjection;
    std::vector<glm::mat4> envMapViews;

    TexuteCube cubeTex;
    CubeRenderPass cubeRenderPass;
    TexuteCube irraTex;
    CubeRenderPass irraRenderPass;

	void IBLSetup(std::string path)
	{
        stbi_set_flip_vertically_on_load(true);
        VkClearValue clearValues = { 0 };
        fence fence;
        // get CommandBuffer
        auto& commandBuffer = graphicsBase::Plus().CommandBuffer_Transfer();
        envMapViews.resize(6);
        // Vulkan cube map face order: +X, -X, +Y, -Y, +Z, -Z
        envMapViews[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));  // +X
        envMapViews[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // -X
        envMapViews[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));  // +Y
        envMapViews[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)); // -Y
        envMapViews[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));  // +Z
        envMapViews[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)); // -Z
        envMapProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        cube.LoadCube();
	    hdrTexture.Create(
			path.c_str(),
			VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_FORMAT_R32G32B32A32_SFLOAT
		);
        VkSamplerCreateInfo samplerCreateInfo = hdrTexture.SamplerCreateInfo();
        hdrTexSampler.Create(samplerCreateInfo);
        uint32_t mipLevels =
            uint32_t(std::floor(std::log2(std::max(windowSize.height, windowSize.width)))) + 1;

        // uniform data
        uniformBuffer transform(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        transform.TransferData(&envMapProjection, sizeof(glm::mat4), sizeof(glm::mat4));
        VkDescriptorBufferInfo bufferInfoMat = {
        .buffer = transform,
        .offset = 0,
        .range = VK_WHOLE_SIZE
        };
        VkDescriptorImageInfo textureInfo[] = {
            { hdrTexSampler, hdrTexture.ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
        };

        cubeTex.CreateImageView(VK_FORMAT_R16G16B16A16_SFLOAT, this->windowSize);
        cubeRenderPass.CreateFaceViews(cubeTex);
        cubeRenderPass.vertPath = "shaders/texture/LatlongToCube.vert.spv";
        cubeRenderPass.fragPath = "shaders/texture/LatlongToCube.frag.spv";
        std::vector<std::vector<VkDescriptorSetLayoutBinding>> sets = {
            {
                { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT },
                { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
            }
        };
        cubeRenderPass.SetDescriptorSetInfos(sets);
        cubeRenderPass.CreatePipelineRenderPass();


        cubeRenderPass.descriptorSets[0].Write(bufferInfoMat, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
        cubeRenderPass.descriptorSets[0].Write(textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
        // trans IMAGE_LAYOUT to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        cubeTex.TransitionImage(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, cubeTex.mipLevels, 0, 6 },
            0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        commandBuffer.End();
        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, VK_NULL_HANDLE, VK_NULL_HANDLE, fence, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
        fence.WaitAndReset();
        
        for (int i = 0; i < 6; i++) {
            transform.TransferData(&envMapViews[i], sizeof(glm::mat4), 0);
            commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            cubeRenderPass.renderPass.CmdBegin(commandBuffer, cubeRenderPass.framebuffers[i], { {}, cubeRenderPass.windowSize }, clearValues);
            
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubeRenderPass.pipelines[0]);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubeRenderPass.pipelineLayouts[0], 0, 1, cubeRenderPass.descriptorSets[0].Address(), 0, nullptr);
            cube.Draw(commandBuffer);

            cubeRenderPass.renderPass.CmdEnd(commandBuffer);
            commandBuffer.End();
            graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, VK_NULL_HANDLE, VK_NULL_HANDLE, fence, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
            fence.WaitAndReset();
        }
        cubeTex.GenMipMap(commandBuffer);

        irraTex.CreateImageView(VK_FORMAT_R16G16B16A16_SFLOAT, { 32, 32 });
        irraRenderPass.CreateFaceViews(irraTex);
        irraRenderPass.vertPath = "shaders/texture/IrradianceIBL.vert.spv";
        irraRenderPass.fragPath = "shaders/texture/IrradianceIBL.frag.spv";
        irraRenderPass.SetDescriptorSetInfos(sets);
        irraRenderPass.CreatePipelineRenderPass();

        VkDescriptorImageInfo textureCubeInfo[] = {
            { cubeTex.sample, cubeTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}
        };
        irraRenderPass.descriptorSets[0].Write(bufferInfoMat, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
        irraRenderPass.descriptorSets[0].Write(textureCubeInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
        // trans IMAGE_LAYOUT to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        irraTex.TransitionImage(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, irraTex.mipLevels, 0, 6 },
            0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        commandBuffer.End();
        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, VK_NULL_HANDLE, VK_NULL_HANDLE, fence, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
        fence.WaitAndReset();

        for (int i = 0; i < 6; i++) {
            transform.TransferData(&envMapViews[i], sizeof(glm::mat4), 0);
            commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            irraRenderPass.renderPass.CmdBegin(commandBuffer, irraRenderPass.framebuffers[i], { {}, irraRenderPass.windowSize }, clearValues);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, irraRenderPass.pipelines[0]);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, irraRenderPass.pipelineLayouts[0], 0, 1, irraRenderPass.descriptorSets[0].Address(), 0, nullptr);
            cube.Draw(commandBuffer);

            irraRenderPass.renderPass.CmdEnd(commandBuffer);
            commandBuffer.End();
            graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, VK_NULL_HANDLE, VK_NULL_HANDLE, fence, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
            fence.WaitAndReset();
        }
        irraTex.GenMipMap(commandBuffer);
	}
};