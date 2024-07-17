#pragma once
#include "data/hash_string.h"

// jeGraphic forward declares
namespace vk
{
    class ShaderModule;
} // vk

namespace graphics
{

enum class ShaderStageBits : u32
{
    FRAGMENT,
    VERTEX,
};

using ShaderStageFlags = std::underlying_type_t<ShaderStageBits>;

struct ShaderResource
{
    mtl::hash_string m_name;
    ShaderStageFlags stages;
};

class Shader
{
public:
private:
};

} // graphics