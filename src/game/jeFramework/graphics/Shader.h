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

namespace fw
{
namespace gfx
{

struct ShaderDefinition
{
    mtl::hash_string name;

    const char* vertex;
    const char* fragment;

    const char* metadata;
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

    Shader(vk::RenderContext& context,
           const ShaderDefinition* definition,
           vk::RenderPass* renderPass,
           u32 subpass);
    ~Shader();

    u32 get_binding_count() const;
    u64 get_binding_size(u32 idx) const;

    const ResourceProxy* lookup_resource(mtl::hash_string location) const;

    vk::PipelineLayout& get_layout() const;
    vk::Pipeline& get_pipeline() const;

    u64 get_descriptor_set_layout_count() const;
    const vk::DescriptorSetLayout& get_descriptor_set_layout(u32 idx = custom_set_idx) const;
    const mtl::hash_string& get_name() const;

    static constexpr u64 custom_set_idx = 3;
private:
    void initialise_layout(const ShaderDefinition* definition);
    void initialise_pipeline(const char* metadataFile, vk::RenderPass* renderpass, u32 subpass);
private:
    mtl::hash_string m_name;

    vk::RenderContext& m_context;

    std::unique_ptr<vk::PipelineLayout> m_layout;
    std::unique_ptr<vk::Pipeline> m_pipeline;

    std::vector<vk::DescriptorSetLayout*> m_descriptorSetLayouts;

    std::unordered_map<mtl::hash_string, ResourceProxy> m_resourceOffsets;
};

} // gfx
} // fw