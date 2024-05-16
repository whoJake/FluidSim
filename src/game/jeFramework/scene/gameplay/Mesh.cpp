#include "Mesh.h"

// template<class V, class T>
Mesh::Mesh(const std::vector<std::vector<V>>& vertices,
           const std::vector<T>& indices) :
    m_vertices(vertices),
    m_indices(indices),
    m_vertexDirty(vertices.size(), true),
    m_indexDirty(true)
{
    if( vertices.size() > 0 )
    {
        m_vertexCount = static_cast<uint32_t>(vertices.front().size());
    }
}

Mesh::Mesh(uint32_t vertexBufferCount,
           const std::vector<T>& indices) :
    m_vertices(vertexBufferCount),
    m_indices(indices),
    m_vertexDirty(vertexBufferCount, true),
    m_indexDirty(true),
    m_vertexCount(0)
{ }

// template<class V, class T>
void Mesh::set_vertex(size_t i, const V& vertex, uint32_t index)
{
    if( m_vertices.at(index).at(i) == vertex )
    {
        return;
    }

    m_vertices.at(index).at(i) = vertex;
    set_vertex_dirty(index);
}

// template<class V, class T>
void Mesh::set_index(size_t i, const T& index)
{
    if( m_indices[i] == index )
    {
        return;
    }

    m_indices[i] = index;
    set_index_dirty();
}

// template<class V, class T>
void Mesh::set_vertices(const std::vector<V>& vertices, uint32_t index)
{
    m_vertices.at(index) = vertices;
    set_vertex_dirty(index);

    if( vertices.size() != m_vertexCount )
    {
        resize_vertex_buffers(static_cast<uint32_t>(vertices.size()));
    }
}

// template<class V, class T>
void Mesh::set_indices(const std::vector<T>& indices)
{
    m_indices = indices;
    set_index_dirty();
}

// template<class V, class T>
const std::vector<Mesh::V>& Mesh::get_vertices(uint32_t index) const
{
    return m_vertices.at(index);
}

// template<class V, class T>
const std::vector<Mesh::T>& Mesh::get_indices() const
{
    return m_indices;
}

uint32_t Mesh::get_vertex_buffer_count() const
{
    return static_cast<uint32_t>(m_vertices.size());
}

// template<class V, class T>
size_t Mesh::get_vertex_count() const
{
    return m_vertexCount;
}

// template<class V, class T>
size_t Mesh::get_index_count() const
{
    return m_indices.size();
}

// template<class V, class T>
size_t Mesh::get_vertex_stride(uint32_t index) const
{
    return sizeof(V);
}

// template<class V, class T>
size_t Mesh::get_vertices_size(uint32_t index) const
{
    return get_vertex_stride(index) * m_vertexCount;
}

size_t Mesh::get_indices_size() const
{
    return sizeof(T) * m_indices.size();
}

// template<class V, class T>
void Mesh::set_vertex_dirty(uint32_t index, bool value)
{
    m_vertexDirty.at(index) = value;
}

void Mesh::set_index_dirty(bool value)
{
    m_indexDirty = value;
}

// template<class V, class T>
bool Mesh::get_vertex_dirty(uint32_t index) const
{
    return m_vertexDirty.at(index);
}

bool Mesh::get_index_dirty() const
{
    return m_indexDirty;
}

void Mesh::resize_vertex_buffers(uint32_t size)
{
    m_vertexCount = size;
    for( size_t i = 0; i < m_vertices.size(); i++ )
    {
        m_vertices.at(i).resize(size);
    }
}

void Mesh::recalculate_normals()
{
    if( m_indices.size() < 3u )
    {
        return;
    }

    for( size_t i = 0; i < m_indices.size() - 2u; i+=3 )
    {
        Vertex a = get_vertices(0).at(m_indices.at(i));
        Vertex b = get_vertices(0).at(m_indices.at(i+1));
        Vertex c = get_vertices(0).at(m_indices.at(i+2));

        glm::vec3 cross = glm::cross(b.position - a.position, c.position - a.position);
        glm::vec3 normal = glm::normalize(cross);
        m_vertices.at(0).at(m_indices.at(i)).normal = normal;
        m_vertices.at(0).at(m_indices.at(i+1)).normal = normal;
        m_vertices.at(0).at(m_indices.at(i+2)).normal = normal;
    }
}