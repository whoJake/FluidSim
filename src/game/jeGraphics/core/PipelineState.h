#pragma once

#include "vkcommon.h"
#include "PipelineLayout.h"
#include "RenderPass.h"

namespace vk
{

struct VertexInputStageState
{
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
};

struct InputAssemblyState
{
    VkPrimitiveTopology topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
    VkBool32 enablePrimitiveRestart{ VK_FALSE };
};

struct TessellationState
{
    uint32_t patchControlPoints;
};

struct ViewportState
{
    uint32_t viewportCount{ 1 };
    uint32_t scissorCount{ 1 };
};

struct RasterizationState
{
    VkBool32 enableDepthClamp{ VK_FALSE };
    VkBool32 enableRasterizerDiscard{ VK_FALSE };
    VkPolygonMode polygonMode{ VK_POLYGON_MODE_LINE };
    VkCullModeFlags cullMode{ VK_CULL_MODE_BACK_BIT };
    VkFrontFace frontFace{ VK_FRONT_FACE_COUNTER_CLOCKWISE };
    VkBool32 enableDepthBias{ VK_FALSE };
    float depthBiasConstantFactor{ 1.f };
    float depthBiasSlopeFactor{ 1.f };
    float lineWidth{ 1.f };
};

struct MultisampleState
{
    VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };
    VkBool32 enableSampleShading{ VK_FALSE };
    float minSampleShading{ 0.f };
    VkSampleMask sampleMask{ 0 };
    VkBool32 enableAlphaToCoverage{ VK_FALSE };
    VkBool32 enableAlphaToOne{ VK_FALSE };
};

struct StencilOpState
{
    VkStencilOp failOp{ VK_STENCIL_OP_REPLACE };
    VkStencilOp passOp{ VK_STENCIL_OP_REPLACE };
    VkStencilOp depthFailOp{ VK_STENCIL_OP_REPLACE };
    VkCompareOp compareOp{ VK_COMPARE_OP_NEVER };
};

struct DepthStencilState
{
    VkBool32 enableDepthTest{ VK_TRUE };
    VkBool32 writeDepth{ VK_TRUE };
    VkCompareOp compareOp{ VK_COMPARE_OP_LESS_OR_EQUAL };
    VkBool32 enableDepthBoundsTest{ VK_FALSE };
    VkBool32 enableStencilTest{ VK_FALSE };
    StencilOpState front{ };
    StencilOpState back{ };
};

struct ColorBlendAttachmentState
{
    VkBool32 blendEnable{ VK_FALSE }; 
    VkBlendFactor srcColorBlendFactor{ VK_BLEND_FACTOR_ONE };
    VkBlendFactor dstColorBlendFactor{ VK_BLEND_FACTOR_ZERO };
    VkBlendOp colorBlendOp{ VK_BLEND_OP_ADD };

    VkBlendFactor srcAlphaBlendFactor{ VK_BLEND_FACTOR_ONE };
    VkBlendFactor dstAlphaBlendFactor{ VK_BLEND_FACTOR_ZERO };
    VkBlendOp alphaBlendOp{ VK_BLEND_OP_ADD };

    VkColorComponentFlags colorWriteMask{ VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
}; 
struct ColorBlendState
{
    VkBool32 enableBlend{ VK_FALSE };
    VkLogicOp logicOp{ VK_LOGIC_OP_CLEAR };
    std::vector<ColorBlendAttachmentState> attachments;
};

class PipelineState
{
public:
    void reset();
    void clear_dirty();
    bool is_dirty() const;

    void set_pipeline_layout(PipelineLayout& layout);

    void set_render_pass(const RenderPass& renderpass);

    void set_subpass_index(uint32_t subpassIndex);

    void set_vertex_input_state(const VertexInputStageState& inputState);

    void set_input_assembly_state(const InputAssemblyState& assemblyState);

    void set_tessellation_state(const TessellationState& tessellationState);

    void set_viewport_state(const ViewportState& viewportState);

    void set_rasterization_state(const RasterizationState& rasterizationState);

    void set_multisample_state(const MultisampleState& multisampleState);

    void set_depth_stencil_state(const DepthStencilState& depthStencilState);

    void set_color_blend_state(const ColorBlendState& colorBlendState);

    const PipelineLayout& get_pipeline_layout() const;

    const RenderPass* get_render_pass() const;

    uint32_t get_subpass_index() const;

    const VertexInputStageState& get_vertex_input_state() const;

    const InputAssemblyState& get_input_assembly_state() const;

    const TessellationState& get_tesselation_state() const;

    const ViewportState& get_viewport_state() const;

    const RasterizationState& get_rasterization_state() const;

    const MultisampleState& get_multisample_state() const;

    const DepthStencilState& get_depth_stencil_state() const;

    const ColorBlendState& get_color_blend_state() const;
private:
    PipelineLayout* m_layout{ nullptr };
    const RenderPass* m_renderPass { nullptr };

    VertexInputStageState m_vertexInputStageState{ };
    InputAssemblyState m_inputAssemblyState{ };
    TessellationState m_tessellationState{ };
    ViewportState m_viewportState{ };
    RasterizationState m_rasterizationState{ };
    MultisampleState m_multisampleState{ };
    DepthStencilState m_depthStencilState{ };
    ColorBlendState m_colorBlendState{ };

    uint32_t m_subpassIndex{ 0 };
    bool m_dirty = false;
};


} // vk
