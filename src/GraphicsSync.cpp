#include "GraphicsSync.h"
#include "GraphicsBase.h"

VkResult GraphicsFence::Create(VkFenceCreateInfo& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult result = vkCreateFence(GraphicsBase::Base()->Device(), &createInfo, nullptr, &handle);
    if (result)
        std::cout << std::format("[ fence ] ERROR\nFailed to create a fence!\nError code: {}\n", int32_t(result));
    return result;
}

GraphicsFence::GraphicsFence(VkFenceCreateInfo& createInfo)
{
    Create(createInfo);
}

GraphicsFence::~GraphicsFence()
{
    DestroyHandleBy(vkDestroyFence);
}

VkResult GraphicsFence::Wait()
{
    VkResult result = vkWaitForFences(GraphicsBase::Base()->Device(), 1, &handle, false, UINT64_MAX);
    if (result)
        std::cout << std::format("[ fence ] ERROR\nFailed to wait for the fence!\nError code: {}\n", int32_t(result));
    return result;
}

VkResult GraphicsFence::Reset()
{
    VkResult result = vkResetFences(GraphicsBase::Base()->Device(), 1, &handle);
    if (result)
        std::cout << std::format("[ fence ] ERROR\nFailed to reset the fence!\nError code: {}\n", int32_t(result));
    return result;
}

VkResult GraphicsFence::WaitAndReset()
{
    VkResult result = Wait();
    result || (result = Reset());
    return result;
}

VkResult GraphicsFence::Status()
{
    VkResult result = vkGetFenceStatus(GraphicsBase::Base()->Device(), handle);
    if (result < 0) //vkGetFenceStatus(...)成功时有两种结果，所以不能仅仅判断result是否非0
        std::cout << std::format("[ fence ] ERROR\nFailed to get the status of the fence!\nError code: {}\n", int32_t(result));
    return result;
}

GraphicsFence::GraphicsFence(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo createInfo = {
            .flags = flags
    };
    Create(createInfo);
}

GraphicsSemaphore::GraphicsSemaphore(VkSemaphoreCreateInfo& createInfo)
{
    Create(createInfo);
}

GraphicsSemaphore::GraphicsSemaphore()
{
    VkSemaphoreCreateInfo createInfo = {};
    Create(createInfo);
}

GraphicsSemaphore::~GraphicsSemaphore()
{
    DestroyHandleBy(vkDestroySemaphore);
}

VkResult GraphicsSemaphore::Create(VkSemaphoreCreateInfo& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(GraphicsBase::Base()->Device(), &createInfo, nullptr, &handle);
    if (result)
        std::cout << std::format("[ semaphore ] ERROR\nFailed to create a semaphore!\nError code: {}\n", int32_t(result));
    return result;
}
