#pragma once
#include "data/hash_string.h"

// jeGraphic forward declares
namespace vk
{
    class RenderContext;
    class ShaderModule;
    class Pipeline;
    class PipelineLayout;
    class ContextBackedBuffer;

    class RenderPass;
} // vk

namespace graphics
{

struct ShaderDefinition
{
    const char* vertex;
    const char* fragment;
    
    const char* metadata;
    vk::RenderPass* renderPass;
    u32 subpass;
};

class Shader
{
public:
    Shader(vk::RenderContext& context, ShaderDefinition* definition);
    ~Shader();
private:
    void initialise_layout(ShaderDefinition* definition);
    void initialise_pipeline(const char* metadataFile, vk::RenderPass* renderpass, u32 subpass);
private:
    vk::RenderContext& m_context;

    std::unique_ptr<vk::PipelineLayout> m_layout;
    std::unique_ptr<vk::Pipeline> m_pipeline;

    struct ResourceProxy
    {
        u32 binding;
        u32 offset;
        u32 size;
    };

    std::unordered_map<mtl::hash_string, ResourceProxy> m_resourceOffsets;
};

} // graphics