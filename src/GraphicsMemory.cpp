#include "GraphicsMemory.h"
#include "GraphicsBase.h"

VkDeviceSize GraphicsMemory::AdjustNonCoherentMemoryRange(VkDeviceSize& size, VkDeviceSize& offset)
{
    const VkDeviceSize& nonCoherentAtomSize = GraphicsBase::Base()->PhysicalDeviceProperties().limits.nonCoherentAtomSize ;
    VkDeviceSize _offset = offset;
    offset = offset / nonCoherentAtomSize * nonCoherentAtomSize;
    size = std::min((size + _offset + nonCoherentAtomSize - 1) / nonCoherentAtomSize * nonCoherentAtomSize, allocationSize) - offset;
    return _offset - offset;
}

GraphicsMemory::GraphicsMemory(VkMemoryAllocateInfo& allocateInfo)
{
    Allocate(allocateInfo);
}

GraphicsMemory::~GraphicsMemory()
{
    DestroyHandleBy(vkFreeMemory);
    allocationSize = 0;
    memoryProperties = 0;
}

VkResult GraphicsMemory::MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset)
{
    VkDeviceSize inverseDeltaOffset;
    if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        inverseDeltaOffset = AdjustNonCoherentMemoryRange(size, offset);
    if (VkResult result = vkMapMemory(GraphicsBase::Base()->Device(), handle, offset, size, 0, &pData)) {
        std::cout << std::format("[ deviceMemory ] ERROR\nFailed to map the memory!\nError code: {}\n", int32_t(result));
        return result;
    }
    if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        pData = static_cast<uint8_t*>(pData) + inverseDeltaOffset;
        VkMappedMemoryRange mappedMemoryRange = {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = handle,
            .offset = offset,
            .size = size
        };
        if (VkResult result = vkInvalidateMappedMemoryRanges(GraphicsBase::Base()->Device(), 1, &mappedMemoryRange)) {
            std::cout << std::format("[ deviceMemory ] ERROR\nFailed to flush the memory!\nError code: {}\n", int32_t(result));
            return result;
        }
    }
    return VK_SUCCESS;
}

VkResult GraphicsMemory::UnmapMemory(VkDeviceSize size, VkDeviceSize offset)
{
    if (!(memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        AdjustNonCoherentMemoryRange(size, offset);
        VkMappedMemoryRange mappedMemoryRange = {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = handle,
            .offset = offset,
            .size = size
        };
        if (VkResult result = vkFlushMappedMemoryRanges(GraphicsBase::Base()->Device(), 1, &mappedMemoryRange)) {
            std::cout << std::format("[ deviceMemory ] ERROR\nFailed to flush the memory!\nError code: {}\n", int32_t(result));
            return result;
        }
    }
    vkUnmapMemory(GraphicsBase::Base()->Device(), handle);
    return VK_SUCCESS;
}

VkResult GraphicsMemory::BufferData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset)
{
    void* pData_dst;
    if (VkResult result = MapMemory(pData_dst, size, offset))
        return result;
    memcpy(pData_dst, pData_src, size_t(size));
    return UnmapMemory(size, offset);
}

VkResult GraphicsMemory::RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset)
{
    void* pData_src;
    if (VkResult result = MapMemory(pData_src, size, offset))
        return result;
    memcpy(pData_dst, pData_src, size_t(size));
    return UnmapMemory(size, offset);
}

VkResult GraphicsMemory::Allocate(VkMemoryAllocateInfo& allocateInfo)
{
    if (allocateInfo.memoryTypeIndex >= GraphicsBase::Base()->PhysicalDeviceMemoryProperties().memoryTypeCount) {
        std::cout << std::format("[ deviceMemory ] ERROR\nInvalid memory type index!\n");
        return VK_RESULT_MAX_ENUM; //没有合适的错误代码，别用VK_ERROR_UNKNOWN
    }
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    if (VkResult result = vkAllocateMemory(GraphicsBase::Base()->Device(), &allocateInfo, nullptr, &handle)) {
        std::cout << std::format("[ deviceMemory ] ERROR\nFailed to allocate memory!\nError code: {}\n", int32_t(result));
        return result;
    }
    //记录实际分配的内存大小
    allocationSize = allocateInfo.allocationSize;
    //取得内存属性
    memoryProperties = GraphicsBase::Base()->PhysicalDeviceMemoryProperties().memoryTypes[allocateInfo.memoryTypeIndex].propertyFlags;
    return VK_SUCCESS;
}
