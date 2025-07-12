#include "GraphicsCmdBuf.h"
#include "GraphicsBase.h"

VkResult GraphicsCmdBuf::Begin(VkCommandBufferUsageFlags usageFlags, VkCommandBufferInheritanceInfo& inheritanceInfo)
{
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = usageFlags,
        .pInheritanceInfo = &inheritanceInfo
    };
    VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
    if (result)
        std::cout << std::format("[ commandBuffer ] ERROR\nFailed to begin a command buffer!\nError code: {}\n", int32_t(result));
    return result;
}

VkResult GraphicsCmdBuf::Begin(VkCommandBufferUsageFlags usageFlags)
{
    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = usageFlags,
    };
    VkResult result = vkBeginCommandBuffer(handle, &beginInfo);
    if (result)
        std::cout << std::format("[ commandBuffer ] ERROR\nFailed to begin a command buffer!\nError code: {}\n", int32_t(result));
    return result;
}

VkResult GraphicsCmdBuf::End()
{
    VkResult result = vkEndCommandBuffer(handle);
    if (result)
        std::cout << std::format("[ commandBuffer ] ERROR\nFailed to end a command buffer!\nError code: {}\n", int32_t(result));
    return result;
}

GraphicsCmdPool::GraphicsCmdPool(VkCommandPoolCreateInfo& createInfo)
{
    Create(createInfo);
}

GraphicsCmdPool::GraphicsCmdPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo createInfo = {
            .flags = flags,
            .queueFamilyIndex = queueFamilyIndex
    };
    Create(createInfo);
}

VkResult GraphicsCmdPool::AllocateBuffer(GraphicsCmdBuf& buffers, VkCommandBufferLevel level) const
{
    VkCommandBufferAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = handle,
            .level = level,
            .commandBufferCount = 1
    };
    VkResult result = vkAllocateCommandBuffers(GraphicsBase::Base()->Device(), &allocateInfo, &buffers.handle);
    if (result)
        std::cout << std::format("[ commandPool ] ERROR\nFailed to allocate command buffers!\nError code: {}\n", int32_t(result));
    return result;
}

void GraphicsCmdPool::FreeBuffer(GraphicsCmdBuf& buffers) const
{
    vkFreeCommandBuffers(GraphicsBase::Base()->Device(), handle, 1, &buffers.handle);
    memset(&buffers.handle, 0, sizeof(VkCommandBuffer));
}

VkResult GraphicsCmdPool::Create(VkCommandPoolCreateInfo& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    VkResult result = vkCreateCommandPool(GraphicsBase::Base()->Device(), &createInfo, nullptr, &handle);
    if (result)
        std::cout << std::format("[ commandPool ] ERROR\nFailed to create a command pool!\nError code: {}\n", int32_t(result));
    return result;
}
