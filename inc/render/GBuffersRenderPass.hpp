#pragma once
#include "VkBase+.h"
#include "common.h"
#include "PipelineRenderPass.hpp"

using namespace vulkan;
class GBuffersRenderPass : public PipelineRenderPass {
private:
	shaderModule vertSh;
	shaderModule fragSh;

	descriptorSetLayout gBufferSetLayout;
	descriptorSetLayout modelInfoSetLayout;
	vulkan::descriptorPool baseSetPool;
	vulkan::descriptorPool modelSetPool;
	descriptorSet baseSet;

	sampler renderSampler;
public:
	VkExtent2D windowSize = { 0, 0 };

	const descriptorSet& GetGBufferSet() { return baseSet; }

	const sampler& GetSampler() { return renderSampler; }

	const VkImageView& GetImageView(int idx)
	{
		return colorAttachments[idx].ImageView();
	}

	virtual bool CreatePipelineRenderPass()
	{
		if (isCreated)
			return false;
		isCreated = true;

		CreateRenderPass();
		CreatePipelineLayout();
		CreatePipeline();
		CreateSampler();

		return true;
	}
	void CreateRenderPass()
	{
		VkAttachmentDescription attachmentDescriptions[] = {
			{// Position
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{// Albedo + Roughness
				.format = VK_FORMAT_R8G8B8A8_UNORM,//The only difference from above
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{// Normals + Metalness
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{// Effects (AO + Velocity)
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{//Depth stencil attachment
				.format = VK_FORMAT_D24_UNORM_S8_UINT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		};
		VkAttachmentReference attachmentReferences[] = {
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			{ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			{ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			{ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			{ 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		};
		VkSubpassDescription subpassDescriptions[] = {
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = GET_ARRAY_NUM(attachmentReferences) - 1,
				.pColorAttachments = attachmentReferences,
				.pDepthStencilAttachment =
					attachmentReferences + (GET_ARRAY_NUM(attachmentReferences) - 1)
			}
		};
		VkSubpassDependency subpassDependencies[] = {
			{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			}
		};
		VkRenderPassCreateInfo renderPassCreateInfo = {
			.attachmentCount = GET_ARRAY_NUM(attachmentDescriptions),
			.pAttachments = attachmentDescriptions,
			.subpassCount = GET_ARRAY_NUM(subpassDescriptions),
			.pSubpasses = subpassDescriptions,
			.dependencyCount = GET_ARRAY_NUM(subpassDependencies),
			.pDependencies = subpassDependencies
		};
		renderPass.Create(renderPassCreateInfo);
		auto CreateFramebuffers = [&] {
			framebuffers.resize(1);
			colorAttachments.resize(4);
			depthStencilAttachments.resize(1);

			// Position
			colorAttachments[0].Create(VK_FORMAT_R16G16B16A16_SFLOAT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
			// Albedo + Roughness
			colorAttachments[1].Create(VK_FORMAT_R8G8B8A8_UNORM, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
			// Normals + Metalness
			colorAttachments[2].Create(VK_FORMAT_R16G16B16A16_SFLOAT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
			// Effects (AO + Velocity)
			colorAttachments[3].Create(VK_FORMAT_R16G16B16A16_SFLOAT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_SAMPLED_BIT);
			depthStencilAttachments[0].Create(VK_FORMAT_D24_UNORM_S8_UINT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
			VkImageView attachments[] = {
				colorAttachments[0].ImageView(),
				colorAttachments[1].ImageView(),
				colorAttachments[2].ImageView(),
				colorAttachments[3].ImageView(),
				depthStencilAttachments[0].ImageView()
			};
			VkFramebufferCreateInfo framebufferCreateInfo = {
				.renderPass = renderPass,
				.attachmentCount = GET_ARRAY_NUM(attachments),
				.pAttachments = attachments,
				.width = windowSize.width,
				.height = windowSize.height,
				.layers = 1
			};
			framebuffers[0].Create(framebufferCreateInfo);
		};
		auto DestroyFramebuffers = [&] {
			for (int i = 0; i < colorAttachments.size(); i++) {
				colorAttachments[i].~colorAttachment();
			}
			for (int i = 0; i < depthStencilAttachments.size(); i++) {
				depthStencilAttachments[i].~depthStencilAttachment();
			}
			framebuffers.clear();
		};
		CreateFramebuffers();
		graphicsBase::Base().AddCallback_CreateSwapchain(name, CreateFramebuffers);
		graphicsBase::Base().AddCallback_DestroySwapchain(name, DestroyFramebuffers);
	}
	void CreatePipelineLayout()
	{
		pipelineLayouts.resize(1);

		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding_gBuffer[] = {
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT },
		};
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding_gModelInfo[] = {
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT },
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
			.bindingCount = GET_ARRAY_NUM(descriptorSetLayoutBinding_gBuffer),
			.pBindings = descriptorSetLayoutBinding_gBuffer
		};
		gBufferSetLayout.Create(descriptorSetLayoutCreateInfo);

		descriptorSetLayoutCreateInfo = {
			.bindingCount = GET_ARRAY_NUM(descriptorSetLayoutBinding_gModelInfo),
			.pBindings = descriptorSetLayoutBinding_gModelInfo
		};
		modelInfoSetLayout.Create(descriptorSetLayoutCreateInfo);

		VkDescriptorSetLayout layouts[] = { gBufferSetLayout, modelInfoSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			.setLayoutCount = GET_ARRAY_NUM(layouts),
			.pSetLayouts = layouts,
		};
		pipelineLayouts[0].Create(pipelineLayoutCreateInfo);


		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		std::map<VkDescriptorType, uint32_t> typeMap;
		for (VkDescriptorSetLayoutBinding element : descriptorSetLayoutBinding_gBuffer) {
			typeMap[element.descriptorType] += element.descriptorCount;
		}
		for (auto [key, val] : typeMap) {
			descriptorPoolSizes.push_back({ key, val });
		}
		baseSetPool.Create(1, descriptorPoolSizes);
		baseSetPool.AllocateSets(baseSet, gBufferSetLayout);

		descriptorPoolSizes.clear();
		typeMap.clear();
		for (VkDescriptorSetLayoutBinding element : descriptorSetLayoutBinding_gModelInfo) {
			typeMap[element.descriptorType] += element.descriptorCount * 1000;
		}
		for (auto [key, val] : typeMap) {
			descriptorPoolSizes.push_back({ key, val });
		}
		modelSetPool.Create(1000, descriptorPoolSizes);
	}
	void CreatePipeline()
	{
		pipelines.resize(1);
		vertSh.Create("shaders/GBuffer.vert.spv");
		fragSh.Create("shaders/GBuffer.frag.spv");
		VkPipelineShaderStageCreateInfo shaderStageCreateInfos_gBuffer[2] = {
			vertSh.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
			fragSh.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		auto Create = [&] {
			//G-buffer
			graphicsPipelineCreateInfoPack pipelineCiPack;
			pipelineCiPack.createInfo.layout = pipelineLayouts[0];
			pipelineCiPack.createInfo.renderPass = renderPass;
			pipelineCiPack.createInfo.subpass = 0;
			pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
			pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
			pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
			pipelineCiPack.vertexInputAttributes.emplace_back(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords));
			pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			//pipelineCiPack.viewports.emplace_back(0.f, float(windowSize.height), float(windowSize.width), -float(windowSize.height), 0.f, 1.f);
			pipelineCiPack.viewports.emplace_back(0.f, 0.0, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
			pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
			pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_FRONT_BIT;
			pipelineCiPack.rasterizationStateCi.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_TRUE;
			pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
			pipelineCiPack.depthStencilStateCi.depthCompareOp = VK_COMPARE_OP_LESS;
			pipelineCiPack.colorBlendAttachmentStates.resize(4);
			pipelineCiPack.colorBlendAttachmentStates[0].colorWriteMask = 0b1111;
			pipelineCiPack.colorBlendAttachmentStates[1].colorWriteMask = 0b1111;
			pipelineCiPack.colorBlendAttachmentStates[2].colorWriteMask = 0b1111;
			pipelineCiPack.colorBlendAttachmentStates[3].colorWriteMask = 0b1111;
			pipelineCiPack.UpdateAllArrays();
			pipelineCiPack.createInfo.stageCount = 2;
			pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_gBuffer;
			pipelines[0].Create(pipelineCiPack);
		};
		auto Destroy = [&] {
			pipelines[0].~pipeline();
			pipelines[1].~pipeline();
		};
		graphicsBase::Base().AddCallback_CreateSwapchain(name, Create);
		graphicsBase::Base().AddCallback_DestroySwapchain(name, Destroy);
		Create();
	}

	void AllocateModelSet(descriptorSet& descriptorSet)
	{
		modelSetPool.AllocateSets(descriptorSet, modelInfoSetLayout);
	}
	
	void CreateSampler()
	{
		VkSamplerCreateInfo samplerCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
					.magFilter = VK_FILTER_LINEAR,
					.minFilter = VK_FILTER_LINEAR,
					.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
					.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
					.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
					.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
					.mipLodBias = 0.f,
					.anisotropyEnable = VK_TRUE,
					.maxAnisotropy = graphicsBase::Base().PhysicalDeviceProperties().limits.maxSamplerAnisotropy,
					.compareEnable = VK_FALSE,
					.compareOp = VK_COMPARE_OP_ALWAYS,
					.minLod = 0.f,
					.maxLod = 0.f,
					.borderColor = {},
					.unnormalizedCoordinates = VK_FALSE
		};
		renderSampler.Create(samplerCreateInfo);
	}
};