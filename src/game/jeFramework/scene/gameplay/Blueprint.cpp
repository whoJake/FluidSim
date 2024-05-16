#include "Blueprint.h"

Blueprint::Blueprint(const std::string_view& name, uint32_t vertexBufferCount) :
    m_name(name),
    m_mesh(std::make_unique<Mesh>(vertexBufferCount)),
    m_id(static_cast<bpid_t>(std::hash<std::string_view>()(name)))
{ }

Blueprint::Blueprint(Blueprint&& other) :
    m_name(other.m_name),
    m_boundingBox(other.m_boundingBox),
    m_mesh(std::move(other.m_mesh)),
    m_id(other.m_id)
{ }

Blueprint::~Blueprint()
{ }

const std::string_view& Blueprint::get_name() const
{
    return m_name;
}

AABoundingBox<> Blueprint::get_bounds() const
{
    return m_boundingBox;
}

Mesh& Blueprint::mesh()
{
    return *m_mesh;
}

bpid_t Blueprint::get_id() const
{
    return m_id;
}