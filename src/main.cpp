#include "GlfwGeneral.hpp"
#include "RenderPassWrapper.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 0.0;
float lastY = 0.0;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees, to rotate the camera to the right
float pitch = 0.0f; // pitch is initialized to 0.0 degrees, to keep the camera level
Camera camera;

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float cameraSpeed = 5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.cameraPos += cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.cameraPos -= cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera.cameraFront = glm::normalize(front);
}

int main()
{
	if (!InitializeWindow(vulkan::defaultWindowSize))
		return -1;
    glfwSetCursorPosCallback(pWindow, mouse_callback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    camera.SetProj(vulkan::defaultWindowSize.width, vulkan::defaultWindowSize.height);
    Mesh cube;
    cube.LoadCube();
    cube.SetWorldPos();

    DeferredRenderPassWrapper dfRp;
    dfRp.CreateRenderPass();
    dfRp.CreatePipelineLayout();
    dfRp.CreatePipeline();

    uniformBuffer uniformBufferMat(sizeof(glm::mat4) * 3, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    uniformBufferMat.TransferData(&cube.model, sizeof(glm::mat4), 0);
    uniformBufferMat.TransferData(&camera.view, sizeof(glm::mat4), sizeof(glm::mat4));
    uniformBufferMat.TransferData(&camera.projection, sizeof(glm::mat4), sizeof(glm::mat4) * 2);

    fence fence;
	semaphore semaphore_imageIsAvailable;
	semaphore semaphore_renderingIsOver;
	commandBuffer commandBuffer;
	commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	commandPool.AllocateBuffers(commandBuffer);

    VkDescriptorPoolSize descriptorPoolSizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3 }
    };
    descriptorPool descriptorPool(2, descriptorPoolSizes);
    descriptorSet descriptorSet_transform;
    descriptorSet descriptorSet_inputAttach;
    descriptorPool.AllocateSets(descriptorSet_transform, dfRp.descriptorSetLayout_gBuffer);
    descriptorPool.AllocateSets(descriptorSet_inputAttach, dfRp.descriptorSetLayout_composition);
    VkDescriptorBufferInfo bufferInfoMat = {
        .buffer = uniformBufferMat,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    descriptorSet_transform.Write(bufferInfoMat, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    auto UpdateDescriptorSet_InputAttachments = [&] {
        VkDescriptorImageInfo imageInfos[] = {
        	{ VK_NULL_HANDLE, dfRp.mColorAttachments[0].ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
        	{ VK_NULL_HANDLE, dfRp.mColorAttachments[1].ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
            { VK_NULL_HANDLE, dfRp.mColorAttachments[2].ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
        };
        descriptorSet_inputAttach.Write(imageInfos, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0, 0);
    };
    graphicsBase::Base().AddCallback_CreateSwapchain(UpdateDescriptorSet_InputAttachments);
    UpdateDescriptorSet_InputAttachments();

    VkClearValue clearValues[] = {
        {.color = {} },
        {.color = {} },
        {.color = {} },
        {.color = {} },
        {.depthStencil = { 1.f, 0 } }
    };

    VkExtent2D windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;
    while (!glfwWindowShouldClose(pWindow)) {
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();
        processInput(pWindow);

        camera.updateView();
        // 更新view矩阵
        uniformBufferMat.TransferData(&camera.view, sizeof(glm::mat4), sizeof(glm::mat4));

        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
        auto i = graphicsBase::Base().CurrentImageIndex();
        
        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        dfRp.mRenderPass.CmdBegin(commandBuffer, dfRp.mFramebuffers[i], { {}, windowSize }, clearValues);
        //G-buffer
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipeline_gBuffer);
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, cube.vertexBuffer.Address(), &offset);
        vkCmdBindIndexBuffer(commandBuffer, cube.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipelineLayout_gBuffer, 0, 1, descriptorSet_transform.Address(), 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
        dfRp.mRenderPass.CmdNext(commandBuffer);
        //Composition
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipeline_composition);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipelineLayout_composition, 0, 1, descriptorSet_inputAttach.Address(), 0, nullptr);
        vkCmdDraw(commandBuffer, 4, 1, 0, 0);
        dfRp.mRenderPass.CmdEnd(commandBuffer);
        commandBuffer.End();
        
        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable, semaphore_renderingIsOver, fence, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
        graphicsBase::Base().PresentImage(semaphore_renderingIsOver);
        
        glfwPollEvents();
        TitleFps();
        
        fence.WaitAndReset();
    }
    TerminateWindow();
    return 0;
}