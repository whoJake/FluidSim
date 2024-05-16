#include "PipelineState.h"

bool operator==(const VkVertexInputBindingDescription& lhs, const VkVertexInputBindingDescription& rhs)
{
    return std::tie(lhs.binding, lhs.stride, lhs.inputRate)
        == std::tie(rhs.binding, rhs.stride, rhs.inputRate);
}

bool operator==(const VkVertexInputAttributeDescription& lhs, const VkVertexInputAttributeDescription& rhs)
{
    return std::tie(lhs.binding, lhs.format, lhs.location, lhs.offset)
        == std::tie(rhs.binding, rhs.format, rhs.location, rhs.offset);
}

namespace vk
{

bool operator==(const ColorBlendAttachmentState& lhs, const ColorBlendAttachmentState& rhs)
{
    return std::tie(lhs.blendEnable, lhs.srcColorBlendFactor, lhs.dstColorBlendFactor, lhs.colorBlendOp, lhs.srcAlphaBlendFactor, lhs.dstAlphaBlendFactor, lhs.alphaBlendOp, lhs.colorWriteMask)
        == std::tie(rhs.blendEnable, rhs.srcColorBlendFactor, rhs.dstColorBlendFactor, rhs.colorBlendOp, rhs.srcAlphaBlendFactor, rhs.dstAlphaBlendFactor, rhs.alphaBlendOp, rhs.colorWriteMask);
}

bool operator!=(const VertexInputStageState& lhs, const VertexInputStageState& rhs)
{
    return lhs.bindings != rhs.bindings || lhs.attributes != rhs.attributes;
}

bool operator!=(const InputAssemblyState& lhs, const InputAssemblyState& rhs)
{
    return std::tie(lhs.topology, lhs.enablePrimitiveRestart) != std::tie(rhs.topology, rhs.enablePrimitiveRestart);
}

bool operator!=(const TessellationState& lhs, const TessellationState& rhs)
{
    return lhs.patchControlPoints != rhs.patchControlPoints;
}

bool operator!=(const ViewportState& lhs, const ViewportState& rhs)
{
    return std::tie(lhs.viewportCount, lhs.scissorCount) != std::tie(rhs.viewportCount, rhs.scissorCount);
}

bool operator!=(const RasterizationState& lhs, const RasterizationState& rhs)
{
    return std::tie(lhs.enableDepthClamp, lhs.enableRasterizerDiscard, lhs.polygonMode, lhs.cullMode, lhs.frontFace, lhs.enableDepthBias, lhs.depthBiasConstantFactor, lhs.depthBiasSlopeFactor, lhs.lineWidth)
        != std::tie(rhs.enableDepthClamp, rhs.enableRasterizerDiscard, rhs.polygonMode, rhs.cullMode, rhs.frontFace, rhs.enableDepthBias, rhs.depthBiasConstantFactor, rhs.depthBiasSlopeFactor, rhs.lineWidth);
}

bool operator!=(const MultisampleState& lhs, const MultisampleState& rhs)
{
    return std::tie(lhs.sampleCount, lhs.enableSampleShading, lhs.minSampleShading, lhs.sampleMask, lhs.enableAlphaToCoverage, lhs.enableAlphaToOne)
        != std::tie(rhs.sampleCount, rhs.enableSampleShading, rhs.minSampleShading, rhs.sampleMask, rhs.enableAlphaToCoverage, rhs.enableAlphaToOne);
}

bool operator!=(const StencilOpState& lhs, const StencilOpState& rhs)
{
    return std::tie(lhs.failOp, lhs.passOp, lhs.depthFailOp, lhs.compareOp)
        != std::tie(rhs.failOp, rhs.passOp, rhs.depthFailOp, rhs.compareOp);
}

bool operator!=(const DepthStencilState& lhs, const DepthStencilState& rhs)
{
    return std::tie(lhs.enableDepthTest, lhs.writeDepth, lhs.compareOp, lhs.enableDepthBoundsTest, lhs.enableStencilTest)
        != std::tie(rhs.enableDepthTest, rhs.writeDepth, rhs.compareOp, rhs.enableDepthBoundsTest, rhs.enableStencilTest)
        || lhs.back != rhs.back || lhs.front != lhs.front;
}

bool operator!=(const ColorBlendAttachmentState& lhs, const ColorBlendAttachmentState& rhs)
{
    return std::tie(lhs.blendEnable, lhs.srcColorBlendFactor, lhs.dstColorBlendFactor, lhs.colorBlendOp, lhs.srcAlphaBlendFactor, lhs.dstAlphaBlendFactor, lhs.alphaBlendOp, lhs.colorWriteMask)
        != std::tie(rhs.blendEnable, rhs.srcColorBlendFactor, rhs.dstColorBlendFactor, rhs.colorBlendOp, rhs.srcAlphaBlendFactor, rhs.dstAlphaBlendFactor, rhs.alphaBlendOp, rhs.colorWriteMask);
}

bool operator!=(const ColorBlendState& lhs, const ColorBlendState& rhs)
{
    if( lhs.attachments.size() != rhs.attachments.size() )
    {
        return true;
    }

    if( std::tie(lhs.enableBlend, lhs.logicOp) != std::tie(rhs.enableBlend, rhs.logicOp) )
    {
        return true;
    }

    for( size_t i = 0; i < lhs.attachments.size(); i++ )
    {
        if( lhs.attachments[i] != rhs.attachments[i] )
        {
            return true;
        }
    }
    return false;
}

void PipelineState::reset()
{
    clear_dirty();
    m_layout = nullptr;
    m_vertexInputStageState = { };
    m_inputAssemblyState = { };
    m_tessellationState = { };
    m_viewportState = { };
    m_rasterizationState = { };
    m_multisampleState = { };
    m_depthStencilState = { };
    m_colorBlendState = { };
    m_subpassIndex = 0;
}

void PipelineState::clear_dirty()
{
    m_dirty = false;
}

bool PipelineState::is_dirty() const
{
    return m_dirty;
}

void PipelineState::set_pipeline_layout(PipelineLayout& layout)
{
    if( m_layout )
    {
        if( m_layout->get_handle() != layout.get_handle() )
        {
            m_layout = &layout;
            m_dirty = true;
        }
    }
    else
    {
        m_layout = &layout;
        m_dirty = true;
    }
}

void PipelineState::set_render_pass(const RenderPass& renderPass)
{
    if( m_renderPass )
    {
        if( m_renderPass->get_handle() != renderPass.get_handle() )
        {
            m_renderPass = &renderPass;
            m_dirty = true;
        }
    }
    else
    {
        m_renderPass = &renderPass;
        m_dirty = true;
    }
}

void PipelineState::set_subpass_index(uint32_t subpassIndex)
{
    if( m_subpassIndex != subpassIndex )
    {
        m_subpassIndex = subpassIndex;
        m_dirty = true;
    }
}

void PipelineState::set_vertex_input_state(const VertexInputStageState& inputState)
{
    if( m_vertexInputStageState != inputState )
    {
        m_vertexInputStageState = inputState;
        m_dirty = true;
    }
}

void PipelineState::set_input_assembly_state(const InputAssemblyState& assemblyState)
{
    if( m_inputAssemblyState != assemblyState )
    {
        m_inputAssemblyState = assemblyState;
        m_dirty = true;
    }
}

void PipelineState::set_tessellation_state(const TessellationState& tessellationState)
{
    if( m_tessellationState != tessellationState )
    {
        m_tessellationState = tessellationState;
        m_dirty = true;
    }
}

void PipelineState::set_viewport_state(const ViewportState& viewportState)
{
    if( m_viewportState != viewportState )
    {
        m_viewportState = viewportState;
        m_dirty = true;
    }
}

void PipelineState::set_rasterization_state(const RasterizationState& rasterizationState)
{
    if( m_rasterizationState != rasterizationState )
    {
        m_rasterizationState = rasterizationState;
        m_dirty = true;
    }
}

void PipelineState::set_multisample_state(const MultisampleState& multisampleState)
{
    if( m_multisampleState != multisampleState )
    {
        m_multisampleState = multisampleState;
        m_dirty = true;
    }
}

void PipelineState::set_depth_stencil_state(const DepthStencilState& depthStencilState)
{
    if( m_depthStencilState != depthStencilState )
    {
        m_depthStencilState = depthStencilState;
        m_dirty = true;
    }
}

void PipelineState::set_color_blend_state(const ColorBlendState& colorBlendState)
{
    if( m_colorBlendState != colorBlendState )
    {
        m_colorBlendState = colorBlendState;
        m_dirty = true;
    }
}

const PipelineLayout& PipelineState::get_pipeline_layout() const
{
    return *m_layout;
}

const RenderPass* PipelineState::get_render_pass() const
{
    return m_renderPass;
}

uint32_t PipelineState::get_subpass_index() const
{
    return m_subpassIndex;
}

const VertexInputStageState& PipelineState::get_vertex_input_state() const
{
    return m_vertexInputStageState;
}

const InputAssemblyState& PipelineState::get_input_assembly_state() const
{
    return m_inputAssemblyState;
}

const TessellationState& PipelineState::get_tesselation_state() const
{
    return m_tessellationState;
}

const ViewportState& PipelineState::get_viewport_state() const
{
    return m_viewportState;
}

const RasterizationState& PipelineState::get_rasterization_state() const
{
    return m_rasterizationState;
}

const MultisampleState& PipelineState::get_multisample_state() const
{
    return m_multisampleState;
}

const DepthStencilState& PipelineState::get_depth_stencil_state() const
{
    return m_depthStencilState;
}

const ColorBlendState& PipelineState::get_color_blend_state() const
{
    return m_colorBlendState;
}

} // vk