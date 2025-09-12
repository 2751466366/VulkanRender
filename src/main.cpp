#include "GlfwGeneral.hpp"
#include "GBuffersRenderPass.hpp"
#include "LightingRenderPass.hpp"
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
    //glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetCursorPosCallback(pWindow, mouseCallback);
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    LightObject pointLightObjs[MAX_LIGHT_NUM] = {  };
    LightObject dirLightObjs[MAX_LIGHT_NUM] = {  };
    uint32_t pointLightNum = 0;
    uint32_t dirLightNum = 0;
   /* pointLightObjs[pointLightNum++] = { .position = glm::vec3(10, 0, -10),  .color = glm::vec4(0, 0, 1, 1) };
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
    gBufferRP.windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;
    gBufferRP.CreatePipelineRenderPass();

    LightingRenderPass lightingRP;
    lightingRP.windowSize = graphicsBase::Base().SwapchainCreateInfo().imageExtent;
    lightingRP.CreatePipelineRenderPass();


    Model objectModel;
    objectModel.LoadModel("resource/models/shaderball/shaderball.obj");
    objectModel.LoadTexuture("resource/textures/pbr/rustediron");
    objectModel.SetWorldPos(glm::vec3(0, 0, -30));
    gBufferRP.AllocateModelSet(objectModel.GetDescriptorSet());
    objectModel.InitUnifom();
    objectModel.UpdateDescriptorSet();

    Model objectModel1;
    objectModel1.LoadModel("resource/models/shaderball/shaderball.obj");
    objectModel1.LoadTexuture("resource/textures/pbr/gold");
    objectModel1.SetWorldPos(glm::vec3(30, 0, -30));
    gBufferRP.AllocateModelSet(objectModel1.GetDescriptorSet());
    objectModel1.InitUnifom();
    objectModel1.UpdateDescriptorSet();

    camera.SetProj(gBufferRP.windowSize.width, gBufferRP.windowSize.height);

    uniformBuffer uniformBufferMat(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    uniformBufferMat.TransferData(&camera.view, sizeof(glm::mat4), 0);
    uniformBufferMat.TransferData(&camera.projection, sizeof(glm::mat4), sizeof(glm::mat4));

    uniformBuffer uniformBufferInvMat(sizeof(glm::mat4) * 2, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    uniformBufferInvMat.TransferData(&camera.invView, sizeof(glm::mat4), 0);
    uniformBufferInvMat.TransferData(&camera.invProj, sizeof(glm::mat4), sizeof(glm::mat4));

    uniformBuffer uniformBuffer_lighting(sizeof(glm::mat4), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    uniformBuffer_lighting.TransferData(&camera.view, sizeof(glm::mat4), 0);

    // uniform std140 align
    uint32_t size = 16 + sizeof(LightObject) * MAX_LIGHT_NUM * 2;
    uniformBuffer lightInfoBuf(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    lightInfoBuf.TransferData(&pointLightNum, sizeof(uint32_t), 0);
    lightInfoBuf.TransferData(&dirLightNum,   sizeof(uint32_t), sizeof(uint32_t));
    lightInfoBuf.TransferData(pointLightObjs, sizeof(LightObject) * MAX_LIGHT_NUM, 16);
    lightInfoBuf.TransferData(dirLightObjs,   sizeof(LightObject) * MAX_LIGHT_NUM, 16 + sizeof(LightObject) * MAX_LIGHT_NUM);

    VkDescriptorBufferInfo bufferInfoMat = {
        .buffer = uniformBufferMat,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    VkDescriptorBufferInfo bufferInfoInvMat = {
        .buffer = uniformBufferInvMat,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    VkDescriptorBufferInfo bufferInfoMat_lighting = {
        .buffer = uniformBuffer_lighting,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    VkDescriptorBufferInfo bufferLightInfo = {
        .buffer = lightInfoBuf,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };
    const descriptorSet& gBufferSetLayout = gBufferRP.GetGBufferSet();
    const descriptorSet& compositionLayout = lightingRP.GetCompositionSet();

    gBufferSetLayout.Write(bufferInfoMat, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);

    VkDescriptorImageInfo gBufferInfo;
    gBufferInfo.sampler = gBufferRP.GetSampler();
    gBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    for (int i = 0; i < 4; i++) {
        gBufferInfo.imageView = gBufferRP.GetImageView(i);
        compositionLayout.Write(gBufferInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i, 0);
    }

    VkDescriptorImageInfo textureInfo =
    { iblWrapper.cubeTex.sample, iblWrapper.cubeTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionLayout.Write(textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, 0);
    textureInfo =
    { iblWrapper.irraTex.sample, iblWrapper.irraTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionLayout.Write(textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, 0);
    textureInfo =
    { iblWrapper.prefilterTex.sample, iblWrapper.prefilterTex.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionLayout.Write(textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, 0);
    textureInfo =
    { iblWrapper.integrateBRDFTexSampler, iblWrapper.integrateBRDFTex.ImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    compositionLayout.Write(textureInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7, 0);

    compositionLayout.Write(bufferInfoInvMat, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8);
    compositionLayout.Write(bufferInfoMat_lighting, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 9);
    compositionLayout.Write(bufferLightInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10);

    VkClearValue gbufferClearValues[] = {
        {.color = {0.0, 0.0, 0.0, 1.0} }, // w represents depth
        {.color = {} },
        {.color = {} },
        {.color = {} },
        {.depthStencil = { 1.f, 0 } }
    };
    VkClearValue lightingClearValues = { .color = {} };


    fence fence;
    semaphore semaphore_imageIsAvailable;
    semaphore semaphore_renderingIsOver;
    commandBuffer commandBuffer;
    commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandPool.AllocateBuffers(commandBuffer);

    while (!glfwWindowShouldClose(pWindow)) {
        while (glfwGetWindowAttrib(pWindow, GLFW_ICONIFIED))
            glfwPollEvents();
        processInput(pWindow);

        camera.updateView();
        uniformBufferMat.TransferData(&camera.view, sizeof(glm::mat4), 0);
        uniformBuffer_lighting.TransferData(&camera.view, sizeof(glm::mat4), 0);
        uniformBufferInvMat.TransferData(&camera.invView, sizeof(glm::mat4), 0);

        graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
        auto i = graphicsBase::Base().CurrentImageIndex();
        
        commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        gBufferRP.renderPass.CmdBegin(commandBuffer, gBufferRP.framebuffers[0], { {}, gBufferRP.windowSize }, gbufferClearValues);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelines[0]);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelineLayouts[0], 0, 1, gBufferSetLayout.Address(), 0, nullptr);
        

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelineLayouts[0], 1, 1, objectModel1.GetDescriptorSet().Address(), 0, nullptr);
        objectModel1.Draw(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferRP.pipelineLayouts[0], 1, 1, objectModel.GetDescriptorSet().Address(), 0, nullptr);
        objectModel.Draw(commandBuffer);

        gBufferRP.renderPass.CmdEnd(commandBuffer);

        lightingRP.renderPass.CmdBegin(commandBuffer, lightingRP.framebuffers[i], { {}, lightingRP.windowSize }, lightingClearValues);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingRP.pipelines[0]);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingRP.pipelineLayouts[0], 0, 1, compositionLayout.Address(), 0, nullptr);
        quad.Draw(commandBuffer);

        lightingRP.renderPass.CmdEnd(commandBuffer);
        
        commandBuffer.End();
        
        graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable, semaphore_renderingIsOver, fence);
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
    float cameraSpeed = 30 * deltaTime;
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