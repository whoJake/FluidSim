#pragma once
#include "vkdefines.h"

namespace gfx
{

struct shader_stage_vk
{
    VkShaderModule shader_module;
    std::vector<VkDescriptorSetLayout> set_layouts;
};

struct shader_vk
{
    std::vector<shader_stage_vk> stages;
};

} // gfx