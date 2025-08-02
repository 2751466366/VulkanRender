#pragma once
#include "VkBase+.h"
#include "common.h"

using namespace vulkan;
class PipelineLayoutRecorder {
public:
	std::map<VkDescriptorType, uint32_t> descriptorTypeMap;
	uint32_t setsNum = 0;

	PipelineLayoutRecorder() = default;
	~PipelineLayoutRecorder() = default;

	void AddDescriptorType(VkDescriptorType type, uint32_t num = 1)
	{
		descriptorTypeMap[type] += num;
	}

	void AddSetsNum(uint32_t num = 1)
	{
		setsNum += num;
	}

    std::vector<VkDescriptorPoolSize> GetDescriptorPoolSize()
	{
		std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
		for (auto [key, val] : descriptorTypeMap) {
			descriptorPoolSizes.push_back({ key, val });
		}
		return descriptorPoolSizes;
	}

	uint32_t GetSetsNum()
	{
		return setsNum;
	}
};