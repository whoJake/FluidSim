#pragma once

#include "data/hash_string.h"

#include "core/Buffer.h"

namespace fw
{
namespace gfx
{

enum class DrawDataState : u32
{
    NOT_LOADED = 0,
    LOADING,
    LOADED,
};

struct DrawMesh
{
    u64 offset;
    u32 vertexCount;
    u32 materialIndex;
};

class DrawData
{
public:
    DrawData(std::vector<DrawMesh> meshes, std::vector<mtl::hash_string> materialDependencies);
    ~DrawData() = default;

    void set_data(std::unique_ptr<vk::Buffer>&& buffer);
    vk::Buffer* get_data();

    DrawDataState get_state() const;
    void set_state(DrawDataState state);

    const std::vector<DrawMesh>& get_meshes() const;
private:
    std::unique_ptr<vk::Buffer> m_data;

    std::vector<DrawMesh> m_meshes;
    std::vector<mtl::hash_string> m_materials;
    DrawDataState m_state;
};

} // gfx
} // fw