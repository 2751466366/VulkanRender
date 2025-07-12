#include "GlfwWindowsWrapper.h"
#include "GraphicsSync.h"
#include "GraphicsCmdBuf.h"
#include <sstream>


int main()
{
    if (!GlfwWindowsWrapper::GetInstance()->InitializeWindow({ 1280,720 }))
        return -1;

    GraphicsCmdBuf commandBuffer;
    GraphicsCmdPool commandPool(GraphicsBase::Base()->QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.AllocateBuffer(commandBuffer);

    GraphicsFence fence(VK_FENCE_CREATE_SIGNALED_BIT); //以置位状态创建栅栏
    GraphicsSemaphore semaphore_imageIsAvailable;
    GraphicsSemaphore semaphore_renderingIsOver;
    while (!glfwWindowShouldClose(GlfwWindowsWrapper::GetInstance()->pWindow)) {
        fence.WaitAndReset();
        GraphicsBase::Base()->SwapImage(semaphore_imageIsAvailable.handle);

        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        /*渲染过程，待填充*/
        commandBuffer.End();

        glfwPollEvents();
        GlfwWindowsWrapper::GetInstance()->TitleFps();
    }
    GlfwWindowsWrapper::GetInstance()->TerminateWindow();
    return 0;
}
