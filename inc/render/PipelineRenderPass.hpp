#pragma once
#include "VkBase+.h"
#include "common.h"
#include "PipelineLayoutRecorder.hpp"

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

	PipelineLayoutRecorder recorder;
	descriptorPool descriptorPool;
	std::vector<descriptorSet> descriptorSets;
	PipelineRenderPass()
	{
		name = "PR" + std::to_string(count++);
	}
	~PipelineRenderPass() = default;

	virtual bool CreatePipelineRenderPass() = 0;
};
uint32_t PipelineRenderPass::count = 0;