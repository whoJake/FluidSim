#pragma once
#include "data/hash_string.h"

// jeGraphic forward declares
namespace vk
{
    class RenderContext;
    class ShaderModule;
    class Pipeline;
    class PipelineLayout;
    class DescriptorSetLayout;
    class ContextBackedBuffer;

    class RenderPass;
} // vk

namespace graphics
{

struct ShaderDefinition
{
    std::string name;

    const char* vertex;
    const char* fragment;
    
    const char* metadata;
    vk::RenderPass* renderPass;
    u32 subpass;
};

class Shader
{
public:
    struct ResourceProxy
    {
        u32 binding;
        u32 offset;
        u32 size;
    };

    Shader(vk::RenderContext& context, ShaderDefinition* definition);
    ~Shader();

    u32 get_binding_count() const;
    u64 get_binding_size(u32 idx) const;

    const ResourceProxy* lookup_resource(mtl::hash_string location) const;

    vk::PipelineLayout& get_layout() const;
    vk::Pipeline& get_pipeline() const;
    std::string_view get_name() const;
    const vk::DescriptorSetLayout& get_descriptor_set_layout() const;
private:
    void initialise_layout(ShaderDefinition* definition);
    void initialise_pipeline(const char* metadataFile, vk::RenderPass* renderpass, u32 subpass);
private:
    std::string m_name;

    vk::RenderContext& m_context;

    std::unique_ptr<vk::PipelineLayout> m_layout;
    std::unique_ptr<vk::Pipeline> m_pipeline;
    vk::DescriptorSetLayout* m_descriptorSetLayout;

    std::unordered_map<mtl::hash_string, ResourceProxy> m_resourceOffsets;
};

} // graphics