#include "Pipeline.h"

#include "Device.h"

namespace vk
{

Pipeline::Pipeline(Device& device) :
    Resource(VK_NULL_HANDLE, device)
{ }

Pipeline::Pipeline(Pipeline&& other) :
    Resource(other.m_handle, other.m_device),
    m_state(other.m_state)
{
    other.m_handle = VK_NULL_HANDLE;
}

Pipeline::~Pipeline()
{
    vkDestroyPipeline(get_device().get_handle(), m_handle, nullptr);
}

const PipelineState& Pipeline::get_state() const
{
    return m_state;
}

GraphicsPipeline::GraphicsPipeline(Device&         device,
                                   VkPipelineCache cache,
                                   PipelineState&  state) :
    Pipeline(device)
{
    std::vector<VkShaderModule> shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos;

    for( const ShaderModule* module : state.get_pipeline_layout().get_shader_modules() )
    {
        VkPipelineShaderStageCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        createInfo.stage = module->get_stage();
        createInfo.pName = module->get_entry_point().c_str();

        VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        moduleCreateInfo.codeSize = module->get_binary_size();
        moduleCreateInfo.pCode = module->get_binary_data();

        VkResult moduleResult = vkCreateShaderModule(get_device().get_handle(), &moduleCreateInfo, nullptr, &createInfo.module);
        VK_CHECK(moduleResult, "Failed to create shader module.");

        shaderStageInfos.push_back(createInfo);
        shaderModules.push_back(createInfo.module);
    }

    VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    createInfo.stageCount = to_u32(shaderStageInfos.size());
    createInfo.pStages = shaderStageInfos.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.vertexBindingDescriptionCount = to_u32(state.get_vertex_input_state().bindings.size());
    vertexInputInfo.pVertexBindingDescriptions = state.get_vertex_input_state().bindings.data();

    vertexInputInfo.vertexAttributeDescriptionCount = to_u32(state.get_vertex_input_state().attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = state.get_vertex_input_state().attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssemblyInfo.topology = state.get_input_assembly_state().topology;
    inputAssemblyInfo.primitiveRestartEnable = state.get_input_assembly_state().enablePrimitiveRestart;

    VkPipelineTessellationStateCreateInfo tessellationInfo{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
    tessellationInfo.patchControlPoints = state.get_tesselation_state().patchControlPoints;

    VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportInfo.viewportCount = state.get_viewport_state().viewportCount;
    viewportInfo.scissorCount = state.get_viewport_state().scissorCount;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizationInfo.depthClampEnable = state.get_rasterization_state().enableDepthClamp;
    rasterizationInfo.rasterizerDiscardEnable = state.get_rasterization_state().enableRasterizerDiscard;

    rasterizationInfo.polygonMode = state.get_rasterization_state().polygonMode;
    rasterizationInfo.cullMode = state.get_rasterization_state().cullMode;
    rasterizationInfo.frontFace = state.get_rasterization_state().frontFace;

    rasterizationInfo.depthBiasEnable = state.get_rasterization_state().enableDepthBias;
    rasterizationInfo.depthBiasConstantFactor = state.get_rasterization_state().depthBiasConstantFactor;
    rasterizationInfo.depthBiasSlopeFactor = state.get_rasterization_state().depthBiasSlopeFactor;

    rasterizationInfo.lineWidth = state.get_rasterization_state().lineWidth;

    VkPipelineMultisampleStateCreateInfo multisampleInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampleInfo.rasterizationSamples = state.get_multisample_state().sampleCount;
    multisampleInfo.sampleShadingEnable = state.get_multisample_state().enableSampleShading;
    multisampleInfo.minSampleShading = state.get_multisample_state().minSampleShading;

    if( state.get_multisample_state().sampleMask )
    {
        multisampleInfo.pSampleMask = &state.get_multisample_state().sampleMask;
    }

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencilInfo.depthTestEnable = state.get_depth_stencil_state().enableDepthTest;
    depthStencilInfo.depthWriteEnable = state.get_depth_stencil_state().writeDepth;
    depthStencilInfo.depthCompareOp = state.get_depth_stencil_state().compareOp;
    depthStencilInfo.depthBoundsTestEnable = state.get_depth_stencil_state().enableDepthBoundsTest;
    depthStencilInfo.stencilTestEnable = state.get_depth_stencil_state().enableStencilTest;

    depthStencilInfo.front.failOp = state.get_depth_stencil_state().front.failOp;
    depthStencilInfo.front.passOp = state.get_depth_stencil_state().front.passOp;
    depthStencilInfo.front.depthFailOp = state.get_depth_stencil_state().front.depthFailOp;
    depthStencilInfo.front.compareOp = state.get_depth_stencil_state().front.compareOp;
    depthStencilInfo.front.compareMask = 0;
    depthStencilInfo.front.writeMask = 0;
    depthStencilInfo.front.reference = 0;

    depthStencilInfo.back.failOp = state.get_depth_stencil_state().back.failOp;
    depthStencilInfo.back.passOp = state.get_depth_stencil_state().back.passOp;
    depthStencilInfo.back.depthFailOp = state.get_depth_stencil_state().back.depthFailOp;
    depthStencilInfo.back.compareOp = state.get_depth_stencil_state().back.compareOp;
    depthStencilInfo.back.compareMask = 0;
    depthStencilInfo.back.writeMask = 0;
    depthStencilInfo.back.reference = 0;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlendInfo.logicOpEnable = state.get_color_blend_state().enableBlend;
    colorBlendInfo.logicOp = state.get_color_blend_state().logicOp;
    colorBlendInfo.attachmentCount = to_u32(state.get_color_blend_state().attachments.size());
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    for( const ColorBlendAttachmentState& state : state.get_color_blend_state().attachments )
    {
        VkPipelineColorBlendAttachmentState blendState{ };
        blendState.blendEnable = state.blendEnable;
        blendState.srcColorBlendFactor = state.srcColorBlendFactor;
        blendState.dstColorBlendFactor = state.dstColorBlendFactor;
        blendState.colorBlendOp = state.colorBlendOp;
        
        blendState.srcAlphaBlendFactor = state.srcAlphaBlendFactor;
        blendState.dstAlphaBlendFactor = state.dstAlphaBlendFactor;
        blendState.alphaBlendOp = state.alphaBlendOp;

        blendState.colorWriteMask = state.colorWriteMask;

        colorBlendAttachments.push_back(blendState);
    }
    colorBlendInfo.pAttachments = colorBlendAttachments.data();
    colorBlendInfo.blendConstants[0] = 1.f;
    colorBlendInfo.blendConstants[1] = 1.f;
    colorBlendInfo.blendConstants[2] = 1.f;
    colorBlendInfo.blendConstants[3] = 1.f;

    std::array<VkDynamicState, 3> dynamicStates = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_BLEND_CONSTANTS
    };

    VkPipelineDynamicStateCreateInfo dynamicInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicInfo.dynamicStateCount = to_u32(dynamicStates.size());
    dynamicInfo.pDynamicStates = dynamicStates.data();

    createInfo.pVertexInputState = &vertexInputInfo;
    createInfo.pInputAssemblyState = &inputAssemblyInfo;
    createInfo.pTessellationState = &tessellationInfo;
    createInfo.pViewportState = &viewportInfo;
    createInfo.pRasterizationState = &rasterizationInfo;
    createInfo.pMultisampleState = &multisampleInfo;
    createInfo.pDepthStencilState = &depthStencilInfo;
    createInfo.pColorBlendState = &colorBlendInfo;
    createInfo.pDynamicState = &dynamicInfo;

    createInfo.layout = state.get_pipeline_layout().get_handle();
    createInfo.renderPass = state.get_render_pass()->get_handle();
    
    createInfo.subpass = state.get_subpass_index();

    VkResult result = vkCreateGraphicsPipelines(get_device().get_handle(), cache, 1, &createInfo, nullptr, &m_handle);
    VK_CHECK(result, "Failed to create Graphics pipeline.");

    for( VkShaderModule module : shaderModules )
    {
        vkDestroyShaderModule(get_device().get_handle(), module, nullptr);
    }
}

} // vk