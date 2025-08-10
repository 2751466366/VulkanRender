#pragma once
#include "VkBase+.h"
#include "common.h"

using namespace vulkan;
class PipelineRenderPass {
public:
	static uint32_t count;
	std::string name;
	bool isCreated = false;

	renderPass renderPass;
	std::vector<framebuffer> framebuffers;
	std::vector<colorAttachment> colorAttachments;
	std::vector<depthStencilAttachment> depthStencilAttachments;

	std::vector<descriptorSetLayout> descriptorSetLayouts;
	std::vector<pipelineLayout> pipelineLayouts;
	std::vector<pipeline> pipelines;

	descriptorPool descriptorPool;
	std::vector<descriptorSet> descriptorSets;
	std::map<VkDescriptorType, uint32_t> descriptorTypeMap;
	uint32_t setsNum = 0;

	PipelineRenderPass()
	{
		name = "PR" + std::to_string(count++);
	}
	~PipelineRenderPass()
	{
		graphicsBase::Base().RemoveCallback(name);
	}

	virtual bool CreatePipelineRenderPass()
	{
		std::cout << "no implememt\n";
		return false;
	}

	void AddDescriptorType(VkDescriptorType type, uint32_t num = 1)
	{
		descriptorTypeMap[type] += num;
	}

	void AddSetsNum(uint32_t num = 1)
	{
		setsNum += num;
	}

	void CreateDescriptor()
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		for (auto [key, val] : descriptorTypeMap) {
			descriptorPoolSizes.push_back({ key, val });
		}

		descriptorPool.Create(setsNum, descriptorPoolSizes);
		descriptorSets.resize(descriptorSetLayouts.size());
		for (int i = 0; i < descriptorSets.size(); i++) {
			descriptorPool.AllocateSets(descriptorSets[i], descriptorSetLayouts[i]);
		}
	}
};
uint32_t PipelineRenderPass::count = 0;