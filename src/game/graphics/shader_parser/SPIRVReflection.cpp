#include "SPIRVReflection.h"
#include <limits>

namespace vk
{

#pragma region read_resource_X

inline static void read_resource_vec_size(const spirv_cross::Compiler& compiler,
                                          const spirv_cross::Resource& resource,
                                          ShaderResource*              shaderResource)
{
    const auto& type = compiler.get_type_from_variable(resource.id);
    shaderResource->vecSize = type.vecsize;
    shaderResource->columns = type.columns;
}

inline static void read_resource_array_size(const spirv_cross::Compiler& compiler,
                                            const spirv_cross::Resource& resource,
                                            ShaderResource*              shaderResource)
{
    const auto& type = compiler.get_type_from_variable(resource.id);
    shaderResource->arraySize = type.array.size()
        ? type.array[0]
        : 1;
}

inline static void read_resource_struct_size(const spirv_cross::Compiler& compiler,
                                             const spirv_cross::Resource& resource,
                                             ShaderResource*              shaderResource)
{
    const auto& type = compiler.get_type_from_variable(resource.id);
    shaderResource->stride = to_u32(compiler.get_declared_struct_size(type));
}

inline static void read_resource_size(const spirv_cross::Compiler&     compiler,
                                      const spirv_cross::SPIRConstant& constant,
                                      ShaderResource*                  shaderResource)
{
    const auto& type = compiler.get_type(constant.constant_type);

    switch( type.basetype )
    {
    case spirv_cross::SPIRType::BaseType::Boolean:
    case spirv_cross::SPIRType::BaseType::Char:
    case spirv_cross::SPIRType::BaseType::Int:
    case spirv_cross::SPIRType::BaseType::UInt:
    case spirv_cross::SPIRType::BaseType::Float:
        shaderResource->stride = 4;
        break;
    case spirv_cross::SPIRType::BaseType::Int64:
    case spirv_cross::SPIRType::BaseType::UInt64:
    case spirv_cross::SPIRType::BaseType::Double:
        shaderResource->stride = 8;
        break;
    default:
        shaderResource->stride = 0;
        break;
    }
}

inline static void read_resource_struct_members(const spirv_cross::Compiler& compiler,
                                                const spirv_cross::Resource& resource,
                                                ShaderResource*              shaderResource)
{
    uint32_t index = 0;
    while( true )
    {
        std::string memberName = compiler.get_member_name(resource.base_type_id, index);
        if( memberName.empty() )
            return;

        ShaderStructMember member{ };
        member.name = memberName;
        member.offset = compiler.type_struct_member_offset(compiler.get_type(resource.base_type_id), index);
        member.size = u32_cast(compiler.get_declared_struct_member_size(compiler.get_type(resource.base_type_id), index));

        shaderResource->structMembers.push_back(member);
        index++;
    }
}

#pragma endregion read_resource_X

#pragma region read_shader_resource<>
template<ShaderResourceType T>
inline static void read_shader_resource(const spirv_cross::Compiler&        compiler,
                                        const spirv_cross::ShaderResources& spirvResources,
                                        VkShaderStageFlagBits               stage,
                                        std::vector<ShaderResource>&        resources)
{
    QUITFMT("Parsing shader resource {} is not implemented yet.", typeid(T).name);
}

template<>
inline static void read_shader_resource<ShaderResourceType::Input>(const spirv_cross::Compiler&                              compiler,
                                                                   const spirv_cross::ShaderResources&                       spirvResources,
                                                                   VkShaderStageFlagBits                                     stage,
                                                                   std::vector<ShaderResource>&                              resources)
{
    for( auto& resource : spirvResources.stage_inputs )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::Input;

        shaderResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);

        read_resource_vec_size(compiler, resource, &shaderResource);
        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::InputAttachment>(const spirv_cross::Compiler&        compiler,
                                                                             const spirv_cross::ShaderResources& spirvResources,
                                                                             VkShaderStageFlagBits               stage,
                                                                             std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.subpass_inputs )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::InputAttachment;

        shaderResource.inputAttachmentIndex = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
        shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::Output>(const spirv_cross::Compiler&        compiler,
                                                                    const spirv_cross::ShaderResources& spirvResources,
                                                                    VkShaderStageFlagBits               stage,
                                                                    std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.stage_outputs )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::Output;

        shaderResource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);

        read_resource_vec_size(compiler, resource, &shaderResource);
        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::Image>(const spirv_cross::Compiler&        compiler,
                                                                   const spirv_cross::ShaderResources& spirvResources,
                                                                   VkShaderStageFlagBits               stage,
                                                                   std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.separate_images )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::Image;

        shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::ImageSampler>(const spirv_cross::Compiler& compiler,
                                                                   const spirv_cross::ShaderResources& spirvResources,
                                                                   VkShaderStageFlagBits               stage,
                                                                   std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.sampled_images )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::ImageSampler;

        shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::ImageStorage>(const spirv_cross::Compiler&        compiler,
                                                                          const spirv_cross::ShaderResources& spirvResources,
                                                                          VkShaderStageFlagBits               stage,
                                                                          std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.storage_images )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::ImageStorage;

        shaderResource.qualifiers |= ShaderResourceQualifiers::NonReadable;
        shaderResource.qualifiers |= ShaderResourceQualifiers::NonWriteable;
        shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::Sampler>(const spirv_cross::Compiler&        compiler,
                                                                     const spirv_cross::ShaderResources& spirvResources,
                                                                     VkShaderStageFlagBits               stage,
                                                                     std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.separate_samplers )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::Sampler;

        shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::BufferUniform>(const spirv_cross::Compiler&        compiler,
                                                                           const spirv_cross::ShaderResources& spirvResources,
                                                                           VkShaderStageFlagBits               stage,
                                                                           std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.uniform_buffers )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::BufferUniform;

        shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_struct_members(compiler, resource, &shaderResource);
        read_resource_struct_size(compiler, resource, &shaderResource);
        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

template<>
inline static void read_shader_resource<ShaderResourceType::BufferStorage>(const spirv_cross::Compiler&        compiler,
                                                                           const spirv_cross::ShaderResources& spirvResources,
                                                                           VkShaderStageFlagBits               stage,
                                                                           std::vector<ShaderResource>&        resources)
{
    for( auto& resource : spirvResources.storage_buffers )
    {
        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = stage;
        shaderResource.type = ShaderResourceType::BufferStorage;

        shaderResource.qualifiers |= ShaderResourceQualifiers::NonReadable;
        shaderResource.qualifiers |= ShaderResourceQualifiers::NonWriteable;
        shaderResource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        shaderResource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_struct_members(compiler, resource, &shaderResource);
        read_resource_struct_size(compiler, resource, &shaderResource);
        read_resource_array_size(compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}
#pragma endregion read_shader_resource<>

SPIRVReflection::SPIRVReflection(std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage) :
    m_compiler(spirv),
    m_stage(stage),
    m_spirvResources()
{
    // taken from Khronos Vulkan-Samples
    auto options = m_compiler.get_common_options();
    options.enable_420pack_extension = true;
    m_compiler.set_common_options(options);

    m_spirvResources = m_compiler.get_shader_resources();
}

void SPIRVReflection::reflect_shader_resources(std::vector<ShaderResource>* resourceList)
{
    std::vector<ShaderResource>& resources = *resourceList;

    { parse_shader_resources(resources); }
    { parse_push_constants(resources); }
    { parse_specialization_constants(resources); }
}

void SPIRVReflection::parse_shader_resources(std::vector<ShaderResource>& resources) 
{
    { read_shader_resource<ShaderResourceType::Input>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::InputAttachment>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::Output>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::Image>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::ImageSampler>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::ImageStorage>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::Sampler>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::BufferUniform>(m_compiler, m_spirvResources, m_stage, resources); }
    { read_shader_resource<ShaderResourceType::BufferStorage>(m_compiler, m_spirvResources, m_stage, resources); }
}

void SPIRVReflection::parse_push_constants(std::vector<ShaderResource>& resources)
{
    for( auto& resource : m_compiler.get_shader_resources().push_constant_buffers )
    {
        const auto& type = m_compiler.get_type_from_variable(resource.id);

        uint32_t offset = (std::numeric_limits<uint32_t>::max)();

        for( uint32_t i = 0; i < type.member_types.size(); i++ )
        {
            uint32_t memOffset = m_compiler.get_member_decoration(type.self, i, spv::DecorationOffset);
            offset = std::min(offset, memOffset);
        }

        ShaderResource shaderResource{ };
        shaderResource.name = resource.name;
        shaderResource.stages = m_stage;
        shaderResource.type = ShaderResourceType::PushConstant;

        shaderResource.offset = offset;

        read_resource_struct_members(m_compiler, resource, &shaderResource);
        read_resource_struct_size(m_compiler, resource, &shaderResource);

        resources.push_back(shaderResource);
    }
}

void SPIRVReflection::parse_specialization_constants(std::vector<ShaderResource>& resources)
{
    for( auto& resource : m_compiler.get_specialization_constants() )
    {
        auto& value = m_compiler.get_constant(resource.id);

        ShaderResource shaderResource{ };
        shaderResource.name = m_compiler.get_name(resource.id);
        shaderResource.stages = m_stage;
        shaderResource.type = ShaderResourceType::SpecializationConstant;

        shaderResource.offset = 0;
        shaderResource.constantID = resource.constant_id;

        read_resource_size(m_compiler, value, &shaderResource);

        resources.push_back(shaderResource);
    }
}

} // vk