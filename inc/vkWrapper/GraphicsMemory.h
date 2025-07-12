#pragma once
#include <vulkan/vulkan.h>

class GraphicsMemory {
private:
    VkDeviceSize AdjustNonCoherentMemoryRange(VkDeviceSize& size, VkDeviceSize& offset);
public:
    VkDeviceMemory handle = VK_NULL_HANDLE;
    VkDeviceSize allocationSize = 0; //实际分配的内存大小
    VkMemoryPropertyFlags memoryProperties = 0; //内存属性

    GraphicsMemory() = default;
    GraphicsMemory(VkMemoryAllocateInfo& allocateInfo);
    ~GraphicsMemory();
    VkResult MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset = 0);
    VkResult UnmapMemory(VkDeviceSize size, VkDeviceSize offset = 0);
    VkResult BufferData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0);
    VkResult RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset = 0);
    VkResult Allocate(VkMemoryAllocateInfo& allocateInfo);
};

class GraphicsBuffer {
public:
    VkBuffer handle = VK_NULL_HANDLE;

    GraphicsBuffer() = default;
    GraphicsBuffer(VkBufferCreateInfo& createInfo);
    ~GraphicsBuffer();
    VkMemoryAllocateInfo MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const;
    VkResult BindMemory(VkDeviceMemory deviceMemory, VkDeviceSize memoryOffset = 0) const;
    VkResult Create(VkBufferCreateInfo& createInfo);
};

class BufferMemory {
public:
    GraphicsMemory memory;
    GraphicsBuffer buffer;
    BufferMemory() = default;
    BufferMemory(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties);
    ~BufferMemory();
    VkResult Create(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties);
};