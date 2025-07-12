#pragma once
#include <vulkan/vulkan.h>

class GraphicsFence {
public:
	VkFence handle = VK_NULL_HANDLE;
	GraphicsFence(VkFenceCreateInfo& createInfo);
	GraphicsFence(VkFenceCreateFlags flags = 0);
	~GraphicsFence();
	VkResult Wait();
	VkResult Reset();
	VkResult WaitAndReset();
	VkResult Status();
	VkResult Create(VkFenceCreateInfo& createInfo);
};

class GraphicsSemaphore {
public:
	VkSemaphore handle = VK_NULL_HANDLE;
	GraphicsSemaphore(VkSemaphoreCreateInfo& createInfo);
	GraphicsSemaphore();
	~GraphicsSemaphore();
	VkResult Create(VkSemaphoreCreateInfo& createInfo);
};