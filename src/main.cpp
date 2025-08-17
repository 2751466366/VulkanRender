#include "GlfwGeneral.hpp"
#include "DeferredRenderPass.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "TextureCube.hpp"
#include "IBLWrapper.hpp"

float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 0.0;
float lastY = 0.0;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees, to rotate the camera to the right
float pitch = 0.0f; // pitch is initialized to 0.0 degrees, to keep the camera level
Camera camera;

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void LoadSkyBox(TexuteCube& skyBox);

int main()
{
	if (!InitializeWindow(vulkan::defaultWindowSize))
		return -1;
    glfwSetCursorPosCallback(pWindow, mouse_callback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    IBLWrapper iblWrapper;
    iblWrapper.IBLSetup("resource/texture/hdr/loft.hdr");

    TexuteCube skyBox;
    LoadSkyBox(skyBox);

    Mesh cube;
    cube.LoadCube();
    cube.SetWorldPos();

    DeferredRenderPass dfRp;
    dfRp.windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;
    dfRp.CreatePipelineRenderPass();

    camera.SetProj(dfRp.windowSize.width, dfRp.windowSize.height);

    uniformBuffer uniformBufferMat(sizeof(glm::mat4) * 3, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    uniformBufferMat.TransferData(&cube.model, sizeof(glm::mat4), 0);
    uniformBufferMat.TransferData(&camera.view, sizeof(glm::mat4), sizeof(glm::mat4));
    uniformBufferMat.TransferData(&camera.projection, sizeof(glm::mat4), sizeof(glm::mat4) * 2);

    uniformBuffer uniformBufferInvMat(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    uniformBufferInvMat.TransferData(&camera.invView, sizeof(glm::mat4), 0);
    uniformBufferInvMat.TransferData(&camera.invProj, sizeof(glm::mat4), sizeof(glm::mat4));

    fence fence;
	semaphore semaphore_imageIsAvailable;
	semaphore semaphore_renderingIsOver;
	commandBuffer commandBuffer;
	commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	commandPool.AllocateBuffers(commandBuffer);

    VkDescriptorBufferInfo bufferInfoMat = {
        .buffer = uniformBufferMat,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    dfRp.descriptorSets[0].Write(bufferInfoMat, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    auto UpdateDescriptorSet_InputAttachments = [&] {
        VkDescriptorImageInfo imageInfos[] = {
        	{ VK_NULL_HANDLE, dfRp.colorAttachments[0].ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
        	{ VK_NULL_HANDLE, dfRp.colorAttachments[1].ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
            { VK_NULL_HANDLE, dfRp.colorAttachments[2].ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
        };
        dfRp.descriptorSets[1].Write(imageInfos, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0, 0);
        VkDescriptorImageInfo textureInfo[] = {
            { iblWrapper.cubeTex.sample, iblWrapper.cubeTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
        };
        dfRp.descriptorSets[1].Write(textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, 0);
    };
    graphicsBase::Base().AddCallback_CreateSwapchain(dfRp.name, UpdateDescriptorSet_InputAttachments);
    UpdateDescriptorSet_InputAttachments();
    VkDescriptorBufferInfo bufferInfoInvMat = {
        .buffer = uniformBufferInvMat,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    dfRp.descriptorSets[1].Write(bufferInfoInvMat, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);

    VkClearValue clearValues[] = {
        {.color = {} },
        {.color = {0.0, 0.0, 0.0, 1.0} }, // w represents depth
        {.color = {} },
        {.color = {} },
        {.depthStencil = { 1.f, 0 } }
    };

    while (!glfwWindowShouldClose(pWindow)) {
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwWaitEvents();
        processInput(pWindow);

        camera.updateView();
        uniformBufferMat.TransferData(&camera.view, sizeof(glm::mat4), sizeof(glm::mat4));
        uniformBufferInvMat.TransferData(&camera.invView, sizeof(glm::mat4), 0);

        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
        auto i = graphicsBase::Base().CurrentImageIndex();
        
        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        dfRp.renderPass.CmdBegin(commandBuffer, dfRp.framebuffers[i], { {}, dfRp.windowSize }, clearValues);
        //G-buffer
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipelines[0]);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipelineLayouts[0], 0, 1, dfRp.descriptorSets[0].Address(), 0, nullptr);
        cube.Draw(commandBuffer);
        dfRp.renderPass.CmdNext(commandBuffer);
        //Composition
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipelines[1]);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dfRp.pipelineLayouts[1], 0, 1, dfRp.descriptorSets[1].Address(), 0, nullptr);
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        dfRp.renderPass.CmdEnd(commandBuffer);
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

void LoadSkyBox(TexuteCube& skyBox)
{
    std::vector<std::string> paths;
    paths.push_back("resource/texture/skybox/right.jpg");
    paths.push_back("resource/texture/skybox/left.jpg");
    paths.push_back("resource/texture/skybox/top.jpg");
    paths.push_back("resource/texture/skybox/bottom.jpg");
    paths.push_back("resource/texture/skybox/back.jpg");
    paths.push_back("resource/texture/skybox/front.jpg");
    skyBox.CreateWithPictures(paths);
}

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