#pragma once
#include "PipelineRenderPass.hpp"
#include "TextureCube.hpp"

class CubeRenderPass : public PipelineRenderPass {
private:
	shaderModule vertShader;
	shaderModule fragShader;
public:
	VkExtent2D windowSize = { 512, 512 };
	std::vector<vulkan::imageView> faceViews;
	std::string vertPath;
	std::string fragPath;
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetInfos;
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
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
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
		framebuffers.resize(faceViews.size());
		for (int mip = 0; mip < faceViews.size() / 6; mip++) {
			for (int i = 0; i < 6; i++) {
				VkImageView attachments[] = {
					faceViews[mip * 6 + i]
				};
				VkFramebufferCreateInfo framebufferCreateInfo = {
					.renderPass = renderPass,
					.attachmentCount = GET_ARRAY_NUM(attachments),
					.pAttachments = attachments,
					.width = windowSize.width >> mip,
					.height = windowSize.height >> mip,
					.layers = 1
				};
				framebuffers[i].Create(framebufferCreateInfo);
			}
		}
	}
	void CreatePipelineLayout()
	{
		int setNum = descriptorSetInfos.size();
		descriptorSetLayouts.resize(setNum);
		pipelineLayouts.resize(setNum);
		for (int i = 0; i < setNum; i++) {
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
				.bindingCount = (uint32_t)descriptorSetInfos[i].size(),
				.pBindings = descriptorSetInfos[i].data()
			};
			descriptorSetLayouts[i].Create(descriptorSetLayoutCreateInfo);

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
				.setLayoutCount = 1,
				.pSetLayouts = descriptorSetLayouts[i].Address()
			};
			pipelineLayouts[i].Create(pipelineLayoutCreateInfo);

			for (int n = 0; n < descriptorSetInfos[i].size(); n++) {
				AddDescriptorType(descriptorSetInfos[i][n].descriptorType);
			}
			AddSetsNum();
		}
	}
	void CreatePipeline()
	{
		pipelines.resize(1);
		vertShader.Create(vertPath.c_str());
		fragShader.Create(fragPath.c_str());
		VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2] = {
			vertShader.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
			fragShader.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
		};

		graphicsPipelineCreateInfoPack pipelineCiPack;
		pipelineCiPack.createInfo.layout = pipelineLayouts[0];
		pipelineCiPack.createInfo.renderPass = renderPass;
		pipelineCiPack.createInfo.subpass = 0;
		pipelineCiPack.vertexInputBindings.emplace_back(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
		pipelineCiPack.vertexInputAttributes.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
		pipelineCiPack.vertexInputAttributes.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
		pipelineCiPack.vertexInputAttributes.emplace_back(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords));
		pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
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

	void CreateFaceViews(TexuteCube& texCube, bool level = false)
	{
		this->windowSize = texCube.texSize;
		int levelNum = level ? texCube.mipLevels : 1;
		faceViews.resize(levelNum * texCube.mipLevels);
		for (int mip = 0; mip < levelNum; mip++) {
			for (int lay = 0; lay < 6; lay++) {
				VkImageViewCreateInfo viewInfo{};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = texCube.imageMemory.Image();
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = texCube.texFormat;
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = mip;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = lay;
				viewInfo.subresourceRange.layerCount = 1;
				faceViews[mip * 6 + lay].Create(viewInfo);
			}
		}
	}
	void SetDescriptorSetInfos(std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetInfos)
	{
		this->descriptorSetInfos = descriptorSetInfos;
	}
};