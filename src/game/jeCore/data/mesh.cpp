#include "mesh.h"

namespace mtl
{

// submesh

submesh::submesh() :
    m_material(u64_cast(0)),
    m_channels(),
    m_indices(),
    m_vertexCount(0),
    m_indexSize(index_size::size_u16)
{ }

submesh::submesh(submesh&& other) noexcept :
    m_material(std::move(other.m_material)),
    m_channels(std::move(other.m_channels)),
    m_indices(std::move(other.m_indices)),
    m_vertexCount(other.m_vertexCount),
    m_indexSize(other.m_indexSize)
{
    other.m_material = hash_string(u64_cast(0));
    other.m_channels.clear();
    other.m_indices.clear();
    other.m_vertexCount = 0;
}

submesh::submesh(const submesh& other) :
    m_material(other.m_material),
    m_channels(other.m_channels),
    m_indices(other.m_indices),
    m_vertexCount(other.m_vertexCount),
    m_indexSize(other.m_indexSize)
{ }

submesh& submesh::operator=(submesh&& other) noexcept
{
    m_material = std::move(other.m_material);
    other.m_material = hash_string(u64_cast(0));

    m_channels = std::move(other.m_channels);
    other.m_channels.clear();

    m_indices = std::move(other.m_indices);
    other.m_indices.clear();

    m_vertexCount = other.m_vertexCount;
    other.m_vertexCount = 0;

    m_indexSize = other.m_indexSize;
    return *this;
}

submesh& submesh::operator=(const submesh& other)
{
    m_material = other.m_material;
    m_channels = other.m_channels;
    m_indices = other.m_indices;
    m_vertexCount = other.m_vertexCount;
    m_indexSize = other.m_indexSize;
}

submesh::~submesh()
{ }

void submesh::add_channel()
{
    m_channels.push_back(std::vector<glm::vec4>(m_vertexCount));
}

void submesh::remove_channel(u64 idx)
{
    m_channels.erase(m_channels.begin() + idx);
    set_channel_sizes();
}

u64 submesh::get_vertex_count() const
{
    return m_vertexCount;
}

std::vector<glm::vec4>& submesh::get_channel(u64 idx)
{
    set_channel_sizes(); // just incase
    return m_channels[idx];
}

void submesh::set_vertex_count(u64 count)
{
    for( std::vector<glm::vec4>& channel : m_channels )
    {
        channel.resize(count);
    }
    m_vertexCount = count;
}

const hash_string& submesh::get_material_name() const
{
    return m_material;
}

void submesh::set_material_name(const hash_string& name)
{
    m_material = name;
}

index_size submesh::get_index_size() const
{
    return m_indexSize;
}

void submesh::set_index_size(index_size size)
{
    m_indexSize = size;
}

void submesh::set_channel_sizes()
{
    u64 max = u64_min;
    for( const std::vector<glm::vec4>& channel : m_channels )
    {
        max = std::max(max, channel.size());
    }

    set_vertex_count(max);
}

// mesh

mesh::mesh() :
    m_name(u64_cast(0)),
    m_submeshs()
{ }

mesh::mesh(mesh&& other) :
    m_name(std::move(other.m_name)),
    m_submeshs(std::move(other.m_submeshs))
{ }

mesh::mesh(const mesh& other) :
    m_name(other.m_name),
    m_submeshs(other.m_submeshs)
{ }

mesh& mesh::operator=(mesh&& other) noexcept
{
    m_name = std::move(other.m_name);
    m_submeshs = std::move(other.m_submeshs);

    other.m_name = hash_string(u64_cast(0));
    other.m_submeshs.clear();

    return *this;
}

mesh& mesh::operator=(const mesh& other)
{
    m_name = other.m_name;
    m_submeshs = other.m_submeshs;

    return *this;
}

mesh::~mesh()
{ }

u64 mesh::add_submesh()
{
    m_submeshs.emplace_back(submesh{ });
    return m_submeshs.size() - 1u;
}

submesh& mesh::get_submesh(u32 idx)
{
    return m_submeshs[idx];
}

u64 mesh::get_submesh_count() const
{
    return m_submeshs.size();
}

void mesh::set_name(const hash_string& name)
{
    m_name = name;
}

const hash_string& mesh::get_name() const
{
    return m_name;
}

} // mtl