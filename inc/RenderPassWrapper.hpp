#pragma once
#include "VkBase+.h"

using namespace vulkan;
class RenderPassWrapper {
public:
	pipelineLayout mPipelineLayout;
	pipeline mPipeline;
	renderPass mRenderPass;
	std::vector<framebuffer> mFramebuffers;
	RenderPassWrapper() = default;
	~RenderPassWrapper() = default;
	bool CreateDeferredRender()
	{
		VkAttachmentDescription attachmentDescriptions[4] = {
			{//Swapchain attachment
				.format = graphicsBase::Base().SwapchainCreateInfo().imageFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
			{//Deferred normal & z attachment
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,//Or VK_FORMAT_R32G32B32A32_SFLOAT
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{//Deffered albedo & specular attachment
				.format = VK_FORMAT_R8G8B8A8_UNORM,//The only difference from above
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
			{//Depth stencil attachment
				.format = VK_FORMAT_D24_UNORM_S8_UINT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		};
	}
};