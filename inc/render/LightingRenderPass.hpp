#pragma once
#include "VkBase+.h"
#include "common.h"
#include "PipelineRenderPass.hpp"
#include "GlfwGeneral.hpp"


using namespace vulkan;
class LightingRenderPass : public PipelineRenderPass {
private:
	shaderModule vertSh;
	shaderModule fragSh;
public:
	VkExtent2D windowSize = { 0, 0 };

	const descriptorSet& GetCompositionSet() { return descriptorSets[0]; }

	virtual bool CreatePipelineRenderPass()
	{
		if (isCreated)
			return false;
		isCreated = true;

		CreateRenderPass();
		CreatePipelineLayout();
		CreatePipeline();
		CreateDescriptor();

		return true;
	}
	void CreateRenderPass()
	{
		VkAttachmentDescription attachmentDescriptions[] = {
			{//Swapchain attachment
				.format = graphicsBase::Base().SwapchainCreateInfo().imageFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
		};
		VkAttachmentReference attachmentReferences[] = {
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
		};
		VkSubpassDescription subpassDescriptions[] = {
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = GET_ARRAY_NUM(attachmentReferences),
				.pColorAttachments = attachmentReferences,
			}
		};
		VkSubpassDependency subpassDependencies[] = {
			{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = 0,
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
			framebuffers.resize(graphicsBase::Base().SwapchainImageCount());
			windowSize = GetWindowSize();
			VkImageView attachments[] = {
				VK_NULL_HANDLE,
			};
			VkFramebufferCreateInfo framebufferCreateInfo = {
				.renderPass = renderPass,
				.attachmentCount = GET_ARRAY_NUM(attachments),
				.pAttachments = attachments,
				.width = windowSize.width,
				.height = windowSize.height,
				.layers = 1
			};
			for (size_t i = 0; i < graphicsBase::Base().SwapchainImageCount(); i++)
				attachments[0] = graphicsBase::Base().SwapchainImageView(i),
				framebuffers[i].Create(framebufferCreateInfo);
		};
		auto DestroyFramebuffers = [&] {
			framebuffers.clear();
		};
		CreateFramebuffers();
		graphicsBase::Base().AddCallback_CreateSwapchain(name, CreateFramebuffers);
		graphicsBase::Base().AddCallback_DestroySwapchain(name, DestroyFramebuffers);
	}

	void RecordDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& layouts)
	{
		std::map<VkDescriptorType, uint32_t> typeMap;
		for (const VkDescriptorSetLayoutBinding &element : layouts) {
			AddDescriptorType(element.descriptorType, element.descriptorCount);
		}
		AddSetsNum(1);
	}

	void CreatePipelineLayout()
	{
		descriptorSetLayouts.resize(1);
		pipelineLayouts.resize(1);

		descriptorSetLayout& compositionSetLayout = descriptorSetLayouts[0];

		//Composition
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings_composition = {
			// G Buffers
			//{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			// IBL tex
			{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			// transform
			{ 8, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT },
			{ 9, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT },
			// light info
			{ 10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }
		};
		RecordDescriptorSetLayout(descriptorSetLayoutBindings_composition);
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
			.bindingCount = (uint32_t)descriptorSetLayoutBindings_composition.size(),
			.pBindings = descriptorSetLayoutBindings_composition.data()
		};
		compositionSetLayout.Create(descriptorSetLayoutCreateInfo);
		VkDescriptorSetLayout layouts[] = { compositionSetLayout };
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			.setLayoutCount = GET_ARRAY_NUM(layouts),
			.pSetLayouts = layouts,
		};
		pipelineLayouts[0].Create(pipelineLayoutCreateInfo);
	}
	void CreatePipeline()
	{
		pipelines.resize(1);
		vertSh.Create("shaders/Lighting.vert.spv");
		fragSh.Create("shaders/Lighting.frag.spv");
		auto Create = [&] {
			VkPipelineShaderStageCreateInfo shaderStageCreateInfos_composition[2] = {
			vertSh.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
			fragSh.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
			};
			windowSize = GetWindowSize();
			graphicsPipelineCreateInfoPack pipelineCiPack;
			pipelineCiPack.createInfo.layout = pipelineLayouts[0];
			pipelineCiPack.createInfo.renderPass = renderPass;
			pipelineCiPack.createInfo.subpass = 0;
			pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
			pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
			pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
			pipelineCiPack.vertexInputAttributes.emplace_back(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords));
			pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			pipelineCiPack.viewports.emplace_back(0.f, float(windowSize.height), float(windowSize.width), -float(windowSize.height), 0.f, 1.f);
			//pipelineCiPack.viewports.emplace_back(0.f, 0.0, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
			pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
			pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_BACK_BIT;
			pipelineCiPack.rasterizationStateCi.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_TRUE;
			pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
			pipelineCiPack.depthStencilStateCi.depthCompareOp = VK_COMPARE_OP_LESS;
			pipelineCiPack.colorBlendAttachmentStates.resize(1);
			pipelineCiPack.colorBlendAttachmentStates[0].colorWriteMask = 0b1111;
			pipelineCiPack.UpdateAllArrays();
			pipelineCiPack.createInfo.stageCount = 2;
			pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_composition;
			pipelines[0].Create(pipelineCiPack);
		};
		auto Destroy = [&] {
			pipelines[0].~pipeline();
		};
		graphicsBase::Base().AddCallback_CreateSwapchain(name, Create);
		graphicsBase::Base().AddCallback_DestroySwapchain(name, Destroy);
		Create();
	}
};