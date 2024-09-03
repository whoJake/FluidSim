#include "DrawData.h"

namespace fw
{
namespace gfx
{

DrawData::DrawData(std::vector<DrawMesh> meshes, std::vector<mtl::hash_string> materialDependencies) :
    m_meshes(meshes),
    m_materials(materialDependencies),
    m_state(DrawDataState::NOT_LOADED)
{ }

void DrawData::set_data(std::unique_ptr<vk::Buffer>&& buffer)
{
    m_data = std::move(buffer);
}

vk::Buffer* DrawData::get_data()
{
    return m_data.get();
}

DrawDataState DrawData::get_state() const
{
    return m_state;
}

void DrawData::set_state(DrawDataState state)
{
    m_state = state;
}

const std::vector<DrawMesh>& DrawData::get_meshes() const
{
    return m_meshes;
}

} // gfx
} // fw