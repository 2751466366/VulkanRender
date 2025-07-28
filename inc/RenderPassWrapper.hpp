#pragma once
#include "VkBase+.h"
#include "common.h"

using namespace vulkan;
class DeferredRenderPassWrapper {
public:
	renderPass mRenderPass;
	std::vector<framebuffer> mFramebuffers;
	std::vector<colorAttachment> mColorAttachments;
	std::vector<depthStencilAttachment> mDepthStencilAttachments;

	descriptorSetLayout descriptorSetLayout_gBuffer;
	pipelineLayout pipelineLayout_gBuffer;
	pipeline pipeline_gBuffer;
	descriptorSetLayout descriptorSetLayout_composition;
	pipelineLayout pipelineLayout_composition;
	pipeline pipeline_composition;

	bool isCreated = false;
	DeferredRenderPassWrapper() = default;
	~DeferredRenderPassWrapper() = default;
	void CreateRenderPass()
	{
		if (isCreated)
			return;
		isCreated = true;
		VkAttachmentDescription attachmentDescriptions[] = {
			{//Swapchain attachment
				.format = graphicsBase::Base().SwapchainCreateInfo().imageFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
			{// Position
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{// Albedo + Roughness
				.format = VK_FORMAT_R8G8B8A8_UNORM,//The only difference from above
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{// Normals + Metalness
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			//{// Effects (AO + Velocity)
			//	.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			//	.samples = VK_SAMPLE_COUNT_1_BIT,
			//	.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			//	.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			//	.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			//	.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			//	.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{//Depth stencil attachment
				.format = VK_FORMAT_D24_UNORM_S8_UINT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		};
		VkAttachmentReference attachmentReferences_subpass0[] = {
			{ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			{ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			{ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
			{ 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		};
		VkAttachmentReference attachmentReferences_subpass1[] = {
			{ 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{ 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{ 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
		};
		VkSubpassDescription subpassDescriptions[2] = {
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 3,
				.pColorAttachments = attachmentReferences_subpass0,
				.pDepthStencilAttachment = attachmentReferences_subpass0 + 3 },
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.inputAttachmentCount = 3,
				.pInputAttachments = attachmentReferences_subpass1,
				.colorAttachmentCount = 1,
				.pColorAttachments = attachmentReferences_subpass1 + 3 }
		};
		VkSubpassDependency subpassDependencies[2] = {
			{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT },
			{
				.srcSubpass = 0,
				.dstSubpass = 1,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT }
		};
		VkRenderPassCreateInfo renderPassCreateInfo = {
			.attachmentCount = 5,
			.pAttachments = attachmentDescriptions,
			.subpassCount = 2,
			.pSubpasses = subpassDescriptions,
			.dependencyCount = 2,
			.pDependencies = subpassDependencies
		};
		mRenderPass.Create(renderPassCreateInfo);
		auto CreateFramebuffers = [&] {
			VkExtent2D windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;
			mFramebuffers.resize(graphicsBase::Base().SwapchainImageCount());
			mColorAttachments.resize(3);
			mDepthStencilAttachments.resize(1);

			// Position
			mColorAttachments[0].Create(VK_FORMAT_R16G16B16A16_SFLOAT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
			// Albedo + Roughness
			mColorAttachments[1].Create(VK_FORMAT_R8G8B8A8_UNORM, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
			// Normals + Metalness
			mColorAttachments[2].Create(VK_FORMAT_R16G16B16A16_SFLOAT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
			//// Effects (AO + Velocity)
			//mColorAttachments[3].Create(VK_FORMAT_R16G16B16A16_SFLOAT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
			
			mDepthStencilAttachments[0].Create(VK_FORMAT_D24_UNORM_S8_UINT, windowSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
			VkImageView attachments[]= {
				VK_NULL_HANDLE,
				mColorAttachments[0].ImageView(),
				mColorAttachments[1].ImageView(),
				mColorAttachments[2].ImageView(),
				//mColorAttachments[3].ImageView(),
				mDepthStencilAttachments[0].ImageView()
			};
			VkFramebufferCreateInfo framebufferCreateInfo = {
				.renderPass = mRenderPass,
				.attachmentCount = sizeof(attachments) / sizeof(VkImageView),
				.pAttachments = attachments,
				.width = windowSize.width,
				.height = windowSize.height,
				.layers = 1
			};
			for (size_t i = 0; i < graphicsBase::Base().SwapchainImageCount(); i++)
				attachments[0] = graphicsBase::Base().SwapchainImageView(i),
				mFramebuffers[i].Create(framebufferCreateInfo);
		};
		auto DestroyFramebuffers = [&] {
			for (int i = 0; i < mColorAttachments.size(); i++) {
				mColorAttachments[i].~colorAttachment();
			}
			for (int i = 0; i < mDepthStencilAttachments.size(); i++) {
				mDepthStencilAttachments[i].~depthStencilAttachment();
			}
			mFramebuffers.clear();
		};
		CreateFramebuffers();
		graphicsBase::Base().AddCallback_CreateSwapchain(CreateFramebuffers);
		graphicsBase::Base().AddCallback_DestroySwapchain(DestroyFramebuffers);
	}

	void CreatePipelineLayout()
	{
		//G-buffer
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding_gBuffer[] =
		    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT };
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
			.bindingCount = (sizeof(descriptorSetLayoutBinding_gBuffer) / sizeof(VkDescriptorSetLayoutBinding)),
			.pBindings = descriptorSetLayoutBinding_gBuffer
		};
		descriptorSetLayout_gBuffer.Create(descriptorSetLayoutCreateInfo);
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
			.setLayoutCount = 1,
			.pSetLayouts = descriptorSetLayout_gBuffer.Address()
		};
		pipelineLayout_gBuffer.Create(pipelineLayoutCreateInfo);
		//Composition
		VkDescriptorSetLayoutBinding descriptorSetLayoutBindings_composition[] = {
			{ 0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3, VK_SHADER_STAGE_FRAGMENT_BIT },
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT }
		};
		descriptorSetLayoutCreateInfo.bindingCount = (sizeof(descriptorSetLayoutBindings_composition) / sizeof(VkDescriptorSetLayoutBinding));
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings_composition;
		descriptorSetLayout_composition.Create(descriptorSetLayoutCreateInfo);
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayout_composition.Address();
		pipelineLayout_composition.Create(pipelineLayoutCreateInfo);
	}

	void CreatePipeline()
	{
		static shaderModule vert_gBuffer("shaders/GBuffer.vert.spv");
		static shaderModule frag_gBuffer("shaders/GBuffer.frag.spv");
		static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_gBuffer[2] = {
			vert_gBuffer.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
			frag_gBuffer.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		static shaderModule vert_composition("shaders/Composition.vert.spv");
		static shaderModule frag_composition("shaders/Composition.frag.spv");
		static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_composition[2] = {
			vert_composition.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
			frag_composition.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
		};
		auto Create = [&] {
			VkExtent2D windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;
			//G-buffer
			graphicsPipelineCreateInfoPack pipelineCiPack;
			pipelineCiPack.createInfo.layout = pipelineLayout_gBuffer;
			pipelineCiPack.createInfo.renderPass = mRenderPass;
			pipelineCiPack.createInfo.subpass = 0;
			pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
			pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
			pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
			pipelineCiPack.vertexInputAttributes.emplace_back(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords));
			pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
			pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
			pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_BACK_BIT;
			pipelineCiPack.rasterizationStateCi.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_TRUE;
			pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
			pipelineCiPack.depthStencilStateCi.depthCompareOp = VK_COMPARE_OP_LESS;
			pipelineCiPack.colorBlendAttachmentStates.resize(3);
			pipelineCiPack.colorBlendAttachmentStates[0].colorWriteMask = 0b1111;
			pipelineCiPack.colorBlendAttachmentStates[1].colorWriteMask = 0b1111;
			pipelineCiPack.colorBlendAttachmentStates[2].colorWriteMask = 0b1111;
			pipelineCiPack.UpdateAllArrays();
			pipelineCiPack.createInfo.stageCount = 2;
			pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_gBuffer;
			pipeline_gBuffer.Create(pipelineCiPack);
			//Composition
			pipelineCiPack.createInfo.layout = pipelineLayout_composition;
			pipelineCiPack.createInfo.subpass = 1;
			pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_composition;
			pipelineCiPack.vertexInputStateCi.vertexBindingDescriptionCount = 0;
			pipelineCiPack.vertexInputStateCi.vertexAttributeDescriptionCount = 0;
			pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			pipelineCiPack.colorBlendStateCi.attachmentCount = 1;
			pipeline_composition.Create(pipelineCiPack);
		};
		auto Destroy = [&] {
			pipeline_gBuffer.~pipeline();
			pipeline_composition.~pipeline();
		};
		graphicsBase::Base().AddCallback_CreateSwapchain(Create);
		graphicsBase::Base().AddCallback_DestroySwapchain(Destroy);
		Create();
	}
};