#pragma once
#include "PipelineRenderPass.hpp"

class IntegrateBRDFRenderPass : public PipelineRenderPass {
private:
	shaderModule vertShader;
	shaderModule fragShader;
public:
	VkExtent2D windowSize = { 0, 0 };
	const VkImageView* renderTarget;
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
			{
				.format = VK_FORMAT_R16G16_SFLOAT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			}
		};
		VkAttachmentReference attachmentReferences[] = {
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
		};
		VkSubpassDescription subpassDescriptions[] = {
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = GET_ARRAY_NUM(attachmentReferences),
				.pColorAttachments = attachmentReferences
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
		framebuffers.resize(1);
		VkImageView attachments[] = {
			*renderTarget
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
	}
	void CreatePipelineLayout()
	{
		pipelineLayouts.resize(1);
		//VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		//		.bindingCount = (uint32_t)0,
		//};
		//descriptorSetLayouts[0].Create(descriptorSetLayoutCreateInfo);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			.setLayoutCount = 0,
		};
		pipelineLayouts[0].Create(pipelineLayoutCreateInfo);
	}
	void CreatePipeline()
	{
		pipelines.resize(1);
		vertShader.Create("shaders/texture/IntegrateBRDF.vert.spv");
		fragShader.Create("shaders/texture/IntegrateBRDF.frag.spv");
		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2] = {
			vertShader.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
			fragShader.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		graphicsPipelineCreateInfoPack pipelineCiPack;
		float width = float(windowSize.width);
		float height = float(windowSize.height);
		pipelineCiPack.createInfo.layout = pipelineLayouts[0];
		pipelineCiPack.createInfo.renderPass = renderPass;
		pipelineCiPack.createInfo.subpass = 0;
		pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
		pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
		pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
		pipelineCiPack.vertexInputAttributes.emplace_back(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords));
		pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		pipelineCiPack.viewports.emplace_back(0.f, height, width, -height, 0.f, 1.f);
		pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
		/*pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_BACK_BIT;
		pipelineCiPack.rasterizationStateCi.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;*/
		pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_FALSE;
		pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_FALSE;
		pipelineCiPack.depthStencilStateCi.depthCompareOp = VK_COMPARE_OP_LESS;
		pipelineCiPack.colorBlendAttachmentStates.resize(1);
		pipelineCiPack.colorBlendAttachmentStates[0].colorWriteMask = 0b1111;
		pipelineCiPack.UpdateAllArrays();
		pipelineCiPack.createInfo.stageCount = 2;
		pipelineCiPack.createInfo.pStages = shaderStageCreateInfos;
		pipelines[0].Create(pipelineCiPack);
	}
	void SetRenderTarget(texture2d& tex)
	{
		windowSize = tex.Extent();
		renderTarget = tex.AddressOfImageView();
	}
};