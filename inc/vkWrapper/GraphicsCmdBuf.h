#pragma once
#include <vulkan/vulkan.h>

class GraphicsCmdBuf {
public:
	VkCommandBuffer handle = VK_NULL_HANDLE;

	GraphicsCmdBuf() = default;

	VkResult Begin(VkCommandBufferUsageFlags usageFlags, VkCommandBufferInheritanceInfo& inheritanceInfo);
	VkResult Begin(VkCommandBufferUsageFlags usageFlags = 0);
	VkResult End();
};

class GraphicsCmdPool {
public:
	VkCommandPool handle = VK_NULL_HANDLE;

	GraphicsCmdPool() = default;
	GraphicsCmdPool(VkCommandPoolCreateInfo& createInfo);
	GraphicsCmdPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkResult AllocateBuffer(GraphicsCmdBuf &buffers, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
	void FreeBuffer(GraphicsCmdBuf &buffers) const;
	VkResult Create(VkCommandPoolCreateInfo& createInfo);
};