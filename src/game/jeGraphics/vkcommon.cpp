#include "vkcommon.h"

#include "core/ShaderModule.h"
#include "core/RenderPass.h"
#include "rendering/RenderTarget.h"

namespace std
{

size_t hash<vk::Device>::operator()(const vk::Device& device) const
{
    return 0;
}

size_t hash<vk::ShaderModule>::operator()(const vk::ShaderModule& shaderModule) const
{
    size_t result = 0;
    vk::hash_combine(result, shaderModule.get_uid());
    return result;
}

size_t hash<vk::ShaderResource>::operator()(const vk::ShaderResource& resource) const
{
    size_t result = 0;

    if( resource.type == vk::ShaderResourceType::Input ||
        resource.type == vk::ShaderResourceType::Output ||
        resource.type == vk::ShaderResourceType::PushConstant ||
        resource.type == vk::ShaderResourceType::SpecializationConstant )
    {
        return result;
    }

    vk::hash_combine(result, resource.set);
    vk::hash_combine(result, resource.binding);
    vk::hash_combine(result, static_cast<std::underlying_type<vk::ShaderResourceMode>::type>(resource.type));
    vk::hash_combine(result, resource.mode);

    return result;
}

size_t hash<vk::RenderPass>::operator()(const vk::RenderPass& pass) const
{
    size_t result = 0;
    vk::hash_combine(result, pass.get_handle());
    return result;
}

size_t hash<vk::RenderTarget>::operator()(const vk::RenderTarget& target) const
{
    size_t result = 0;

    for( auto& view : target.get_image_views() )
    {
        vk::hash_combine(result, view.get_handle());
        vk::hash_combine(result, view.get_image().get_handle());
    }

    return result;
}

} // std