#pragma once
#include "VkBase+.h"
#include "common.h"

using namespace vulkan;
class TexuteCube {
private:
	void TransitionImage(const commandBuffer& cmd,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		const VkImageSubresourceRange& range,
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
		VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			srcAccessMask,
			dstAccessMask,
			oldLayout,
			newLayout,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			imageMemory.Image(),
			range
		};
		vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0,
			0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}
public:
	imageView imageView;
	imageMemory imageMemory;
	sampler sample;

	TexuteCube() = default;
	~TexuteCube() = default;
	bool CreateWithPictures(const std::vector<std::string>& paths)
	{
		if (paths.size() != 6) {
			std::cout << "wrong size of cubeMap" << std::endl;
			return false;
		}
		// load pictures
		int w, h, comp;
		std::vector<unsigned char*> pixels(6);
		for (int i = 0; i < 6; ++i) {
			pixels[i] = stbi_load(paths[i].c_str(), &w, &h, &comp, 4);
		}
		comp = 4;
		std::cout << "load cubeTex size = [" << w << ", " << h << "] comp= " << comp << std::endl;
		VkFormat format = (comp == 4 ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8_SRGB);
		// create image
		uint32_t mipLevels = uint32_t(std::floor(std::log2(std::max(w, h)))) + 1;
		VkImageCreateInfo imgInfo{};
		imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imgInfo.imageType = VK_IMAGE_TYPE_2D;
		imgInfo.extent.width = w;
		imgInfo.extent.height = h;
		imgInfo.extent.depth = 1;
		imgInfo.mipLevels = mipLevels;
		imgInfo.arrayLayers = 6;
		imgInfo.format = format;
		imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
		imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imgInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		imageMemory.Create(imgInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// create temp buffer for loading all pictures data
		const VkDeviceSize layerSize = w * h * comp;
		const VkDeviceSize totalSize = layerSize * 6;
		bufferMemory tempBuf;
		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = totalSize;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		tempBuf.Create(createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		for (int i = 0; i < 6; ++i) {
			tempBuf.BufferData(i * layerSize, pixels[i], layerSize);
		}

		// get CommandBuffer 
		auto& commandBuffer = graphicsBase::Plus().CommandBuffer_Transfer();
		commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		// trans IMAGE_LAYOUT to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		TransitionImage(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			{ VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 6 },
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		// copy buffer to image (level 0)
		std::vector<VkBufferImageCopy> regions;
		for (uint32_t face = 0; face < 6; ++face) {
			VkBufferImageCopy region{};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = face;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = { (uint32_t)w, (uint32_t)h, 1 };
			region.bufferOffset = face * layerSize;
			regions.push_back(region);
		}
		vkCmdCopyBufferToImage(commandBuffer, tempBuf.Buffer(), imageMemory.Image(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			regions.size(), regions.data());

		// genMipmap
		for (uint32_t level = 1; level < mipLevels; ++level) {
			TransitionImage(commandBuffer,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				{ VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 1, 0, 6 },
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			VkImageBlit blit{};
			blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 0, 6 };
			blit.srcOffsets[1] = { int32_t(w >> (level - 1)),
									int32_t(h >> (level - 1)), 1 };
			blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 6 };
			blit.dstOffsets[1] = { int32_t(w >> level),
									int32_t(h >> level), 1 };

			vkCmdBlitImage(commandBuffer,
				imageMemory.Image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				imageMemory.Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			TransitionImage(commandBuffer,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				{ VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 1, 0, 6 },
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
		TransitionImage(commandBuffer,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			{ VK_IMAGE_ASPECT_COLOR_BIT, mipLevels - 1, 1, 0, 6 },
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		// submit command
		commandBuffer.End();
		graphicsBase::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);

		//cteate ImageView and Sampler
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		view.format = format;
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.levelCount = mipLevels;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 6;
		view.image = imageMemory.Image();
		imageView.Create(view);

		VkSamplerCreateInfo samp{};
		samp.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samp.magFilter = VK_FILTER_LINEAR;
		samp.minFilter = VK_FILTER_LINEAR;
		samp.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samp.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samp.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samp.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samp.minLod = 0.0f;
		samp.maxLod = static_cast<float>(mipLevels);
		sample.Create(samp);
	}
};