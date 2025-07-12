#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class GraphicsPipelineCreateInfoPack {
public:
    VkGraphicsPipelineCreateInfo createInfo =
    { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    //Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    std::vector<VkVertexInputBindingDescription> vertexInputBindings;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
    //Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    //Tessellation
    VkPipelineTessellationStateCreateInfo tessellationStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
    //Viewport
    VkPipelineViewportStateCreateInfo viewportStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    uint32_t dynamicViewportCount = 1;//动态视口/剪裁不会用到上述的vector，因此动态视口和剪裁的个数向这俩变量手动指定
    uint32_t dynamicScissorCount = 1;
    //Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizationStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    //Multisample
    VkPipelineMultisampleStateCreateInfo multisampleStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    //Depth & Stencil
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    //Color Blend
    VkPipelineColorBlendStateCreateInfo colorBlendStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    //Dynamic
    VkPipelineDynamicStateCreateInfo dynamicStateCi =
    { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    std::vector<VkDynamicState> dynamicStates;

    GraphicsPipelineCreateInfoPack() = default;
    void UpdateAllArrays();
};

class PipeLineLayout {
public:
	VkPipelineLayout handle = VK_NULL_HANDLE;
	PipeLineLayout() = default;
	PipeLineLayout(VkPipelineLayoutCreateInfo& createInfo);
	~PipeLineLayout();
	VkResult Create(VkPipelineLayoutCreateInfo& createInfo);
};

class Pipeline {
    VkPipeline handle = VK_NULL_HANDLE;
public:
    Pipeline() = default;
    Pipeline(VkGraphicsPipelineCreateInfo& createInfo);
    Pipeline(VkComputePipelineCreateInfo& createInfo);
    ~Pipeline();
    //Non-const Function
    VkResult Create(VkGraphicsPipelineCreateInfo& createInfo);
    VkResult Create(VkComputePipelineCreateInfo& createInfo);
};