#pragma once
#include "GlfwGeneral.hpp"
#include "GBuffersRenderPass.hpp"
#include "LightingRenderPass.hpp"
#include "LineRenderPass.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "TextureCube.hpp"
#include "IBLWrapper.hpp"
#include "Light.hpp"

const int MAX_LIGHT_NUM = 10;

float lastFrame = 0.0f;
bool firstMouse = true;
bool enableView = false;
bool needResetView = true;
float lastX = 0.0;
float lastY = 0.0;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees, to rotate the camera to the right
float pitch = 0.0f; // pitch is initialized to 0.0 degrees, to keep the camera level
Camera camera;

void processInput(GLFWwindow* window);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void LoadSkyBox(TexuteCube& skyBox);

int main()
{
    if (!InitializeWindow(vulkan::defaultWindowSize))
        return -1;
    stbi_set_flip_vertically_on_load(true);

    glfwSetMouseButtonCallback(pWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(pWindow, mouseCallback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    LightObject pointLightObjs[MAX_LIGHT_NUM] = {  };
    LightObject dirLightObjs[MAX_LIGHT_NUM] = {  };
    uint32_t pointLightNum = 0;
    uint32_t dirLightNum = 0;
    /*pointLightObjs[pointLightNum++] = { .position = glm::vec3(0, -0.5, 0),  .color = glm::vec4(0, 5, 5, 1) };
    pointLightObjs[pointLightNum++] = { .position = glm::vec3(-10, 0, -10), .color = glm::vec4(1, 0, 0, 1) };
    dirLightObjs[dirLightNum++] = { .direction = glm::vec3(0, -1, 0), .color = glm::vec4(1, 1, 1, 1) };*/


    Mesh quad;
    quad.LoadQuad();

    IBLWrapper iblWrapper;
    iblWrapper.IBLSetup("resource/textures/hdr/golden_gate_hills_2k.hdr");
    //iblWrapper.IBLSetup("resource/textures/hdr/canyon.hdr");

    TexuteCube skyBox;
    LoadSkyBox(skyBox);

    GBuffersRenderPass gBufferRP;
    gBufferRP.CreatePipelineRenderPass();

    LightingRenderPass lightingRP;
    lightingRP.CreatePipelineRenderPass();

    LineRenderPass lineRP;
    lineRP.CreatePipelineRenderPass();


    Model objectModel;
    objectModel.LoadModel("resource/models/shaderball/shaderball.obj");
    objectModel.LoadTexuture("resource/textures/pbr/rustediron");
    objectModel.SetWorldPos(glm::vec3(0, 0, -30));
    gBufferRP.AllocateModelSet(objectModel.GetDescriptorSet());
    objectModel.InitUnifom();
    objectModel.UpdateModelMat();
    objectModel.BuildBVH();
    lineRP.BindModelSet("objectModel", objectModel.GetModelMatUniform());

    Model livingroom;
    livingroom.LoadModel("resource/models/livingroom/livingroom.obj");
    livingroom.LoadTexuture("resource/textures/pbr/mixed_brick_wall");
    livingroom.SetWorldPos(glm::vec3(0, -1, 0));
    gBufferRP.AllocateModelSet(livingroom.GetDescriptorSet());
    livingroom.InitUnifom();
    livingroom.UpdateModelMat();

    camera.Init(GetWindowSize());

    uniformBuffer transformData(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    transformData.TransferData(&camera.view, sizeof(glm::mat4), 0);
    transformData.TransferData(&camera.projection, sizeof(glm::mat4), sizeof(glm::mat4));

    uniformBuffer invTransformData(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    invTransformData.TransferData(&camera.invView, sizeof(glm::mat4), 0);
    invTransformData.TransferData(&camera.invProj, sizeof(glm::mat4), sizeof(glm::mat4));

    // uniform std140 align
    uint32_t size = 16 + sizeof(LightObject) * MAX_LIGHT_NUM * 2;
    uniformBuffer lightInfoData(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    lightInfoData.TransferData(&pointLightNum, sizeof(uint32_t), 0);
    lightInfoData.TransferData(&dirLightNum, sizeof(uint32_t), sizeof(uint32_t));
    lightInfoData.TransferData(pointLightObjs, sizeof(LightObject) * MAX_LIGHT_NUM, 16);
    lightInfoData.TransferData(dirLightObjs, sizeof(LightObject) * MAX_LIGHT_NUM, 16 + sizeof(LightObject) * MAX_LIGHT_NUM);

    VkDescriptorBufferInfo transformDataBuffer = {
        .buffer = transformData,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    VkDescriptorBufferInfo invTransformDataBuffer = {
        .buffer = invTransformData,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    VkDescriptorBufferInfo lightInfoDataDesc = {
        .buffer = lightInfoData,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    const descriptorSet& gBufferSet = gBufferRP.GetGBufferSet();
    const descriptorSet& compositionSet = lightingRP.GetCompositionSet();
    const descriptorSet& drawLineSet = lineRP.GetTransformSet();

    gBufferSet.Write(transformDataBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    drawLineSet.Write(transformDataBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);

    auto SetGbuffers = [&] {
        camera.UpdateProj(GetWindowSize());
        VkDescriptorImageInfo gBufferImageDesc;
        gBufferImageDesc.sampler = gBufferRP.GetSampler();
        gBufferImageDesc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        for (int i = 0; i < 4; i++) {
            gBufferImageDesc.imageView = gBufferRP.GetImageView(i);
            compositionSet.Write(gBufferImageDesc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i, 0);
        }
    };
    SetGbuffers();
    graphicsBase::Base().AddCallback_CreateSwapchain(gBufferRP.name, SetGbuffers);

    VkDescriptorImageInfo iblImageDesc =
    { iblWrapper.cubeTex.sample, iblWrapper.cubeTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionSet.Write(iblImageDesc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, 0);
    iblImageDesc =
    { iblWrapper.irraTex.sample, iblWrapper.irraTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionSet.Write(iblImageDesc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, 0);
    iblImageDesc =
    { iblWrapper.prefilterTex.sample, iblWrapper.prefilterTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionSet.Write(iblImageDesc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, 0);
    iblImageDesc =
    { iblWrapper.integrateBRDFTexSampler, iblWrapper.integrateBRDFTex.ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionSet.Write(iblImageDesc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, 0);

    compositionSet.Write(invTransformDataBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8);
    compositionSet.Write(transformDataBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 9);
    compositionSet.Write(lightInfoDataDesc, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10);

    VkClearValue gbufferClearValues[] = {
        {.color = {0.0, 0.0, 0.0, 1.0} }, // w represents depth
        {.color = {} },
        {.color = {} },
        {.color = {} },
        {.depthStencil = { 1.f, 0 } }
    };
    VkClearValue lightingClearValues = { .color = {} };


    fence fence;
    //semaphore semaphore_imageIsAvailable;
    //semaphore semaphore_renderingIsOver;
    static uint32_t currentFrame = 0;
    std::vector<semaphore> semaphore_imageIsAvailable(graphicsBase::Base().SwapchainImageCount());
    std::vector<semaphore> semaphore_renderingIsOver(graphicsBase::Base().SwapchainImageCount());
    commandBuffer commandBuffer;
    commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.AllocateBuffers(commandBuffer);

    while (!glfwWindowShouldClose(pWindow)) {
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwPollEvents();
        processInput(pWindow);

        camera.UpdateView();
        transformData.TransferData(&camera.view, sizeof(glm::mat4), 0);
        invTransformData.TransferData(&camera.invView, sizeof(glm::mat4), 0);

        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable[currentFrame]);
        auto i = graphicsBase::Base().CurrentImageIndex();

        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        gBufferRP.renderPass.CmdBegin(commandBuffer, gBufferRP.framebuffers[0], { {}, gBufferRP.windowSize }, gbufferClearValues);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelines[0]);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelineLayouts[0], 0, 1, gBufferSet.Address(), 0, nullptr);


        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelineLayouts[0], 1, 1, livingroom.GetDescriptorSet().Address(), 0, nullptr);
        livingroom.Draw(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelineLayouts[0], 1, 1, objectModel.GetDescriptorSet().Address(), 0, nullptr);
        objectModel.Draw(commandBuffer);

        gBufferRP.renderPass.CmdEnd(commandBuffer);


        lightingRP.renderPass.CmdBegin(commandBuffer, lightingRP.framebuffers[i], { {}, lightingRP.windowSize }, lightingClearValues);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingRP.pipelines[0]);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingRP.pipelineLayouts[0], 0, 1, compositionSet.Address(), 0, nullptr);
        quad.Draw(commandBuffer);

        lightingRP.renderPass.CmdEnd(commandBuffer);

        VkImageMemoryBarrier bar{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = graphicsBase::Base().SwapchainImage(i),
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        };
        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0, 0, nullptr, 0, nullptr, 1, &bar);

        lineRP.renderPass.CmdBegin(commandBuffer, lineRP.framebuffers[i], { {}, lineRP.windowSize }, lightingClearValues);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lineRP.pipelines[0]);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lineRP.pipelineLayouts[0], 0, 1, drawLineSet.Address(), 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lineRP.pipelineLayouts[0], 1, 1, lineRP.GetModelSet("objectModel").Address(), 0, nullptr);
        objectModel.DrawBVH(commandBuffer, 3);
        lineRP.renderPass.CmdEnd(commandBuffer);

        commandBuffer.End();

        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable[currentFrame], semaphore_renderingIsOver[currentFrame], fence);
        graphicsBase::Base().PresentImage(semaphore_renderingIsOver[currentFrame]);

        glfwPollEvents();
        TitleFps();

        fence.WaitAndReset();
        currentFrame = (currentFrame + 1) % graphicsBase::Base().SwapchainImageCount();
    }
    TerminateWindow();
    return 0;
}

void LoadSkyBox(TexuteCube& skyBox)
{
    std::vector<std::string> paths;
    paths.push_back("resource/textures/skybox/right.jpg");
    paths.push_back("resource/textures/skybox/left.jpg");
    paths.push_back("resource/textures/skybox/top.jpg");
    paths.push_back("resource/textures/skybox/bottom.jpg");
    paths.push_back("resource/textures/skybox/back.jpg");
    paths.push_back("resource/textures/skybox/front.jpg");
    skyBox.CreateWithPictures(paths);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        //std::cout << "Key pressed: " << key << std::endl;
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, true);

        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float cameraSpeed = 10 * deltaTime;
        if (key == GLFW_KEY_W)
            camera.cameraPos += cameraSpeed * camera.cameraFront;
        if (key == GLFW_KEY_S)
            camera.cameraPos -= cameraSpeed * camera.cameraFront;
        if (key == GLFW_KEY_A)
            camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_D)
            camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
    }
    else if (action == GLFW_RELEASE) {
        //std::cout << "Key released: " << key << std::endl;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            enableView = true;
        }
    }
    else if (action == GLFW_RELEASE) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            needResetView = true;
            enableView = false;
        }
    }
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float cameraSpeed = 10 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.cameraPos += cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.cameraPos -= cameraSpeed * camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!enableView)
        return;
    if (needResetView) {
        needResetView = false;
        firstMouse = true;
    }

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

    float sensitivity = 0.1f; // change this value to your liking
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