#pragma once
#include <vulkan/vulkan.h>

class RenderPass {
public:
	RenderPass() = default;
	RenderPass(VkRenderPassCreateInfo& createInfo);
	~RenderPass();
	void CmdBegin(
		VkCommandBuffer commandBuffer,
		VkRenderPassBeginInfo& beginInfo,
		VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
	void CmdBegin(VkCommandBuffer commandBuffer,
		VkFramebuffer Framebuffer,
		VkRect2D renderArea,
		const VkClearValue clearValues,
		VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
	void CmdNext(VkCommandBuffer commandBuffer,
		VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;
	void CmdEnd(VkCommandBuffer commandBuffer) const;
	//Non-const Function
	VkResult Create(VkRenderPassCreateInfo& createInfo);
};

class Framebuffer {
    VkFramebuffer handle = VK_NULL_HANDLE;
public:
    Framebuffer() = default;
	Framebuffer(VkFramebufferCreateInfo& createInfo);
    ~Framebuffer();
	VkResult Create(VkFramebufferCreateInfo& createInfo);
};