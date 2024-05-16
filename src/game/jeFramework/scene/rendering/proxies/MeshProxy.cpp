#include "MeshProxy.h"

MeshProxy::MeshProxy(vk::RenderContext* context, Mesh* mesh) :
    m_mesh(mesh),
    m_indexCount(static_cast<uint32_t>(mesh->get_index_count())),
    m_indexBuffer(*context, mesh->get_indices_size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST)
{
    m_vertexBuffers.reserve(mesh->get_vertex_buffer_count());
    for( uint32_t i = 0; i < mesh->get_vertex_buffer_count(); i++ )
    {
        m_vertexBuffers.emplace_back(*context, mesh->get_vertices_size(i), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    }
}

void MeshProxy::sync()
{
    m_indexCount = static_cast<uint32_t>(m_mesh->get_index_count());

    for( uint32_t i = 0; i < m_mesh->get_vertex_buffer_count(); i++ )
    {
        bool needsRemap;
        if( m_mesh->get_vertex_dirty(i) )
        {
            m_vertexBuffers.at(i).create_new_buffer(m_mesh->get_vertices_size(i));
            needsRemap = true;
            
            m_mesh->set_vertex_dirty(i, false);
        }
        else
        {
            needsRemap = m_vertexBuffers.at(i).carry_over_buffer();
        }

        if( needsRemap )
        {
            uint8_t* data = m_vertexBuffers.at(i).map();
            memcpy(data, m_mesh->get_vertices(i).data(), m_mesh->get_vertices_size(i));
            m_vertexBuffers.at(i).unmap();
        }
    }

    bool needsRemap;
    if( m_mesh->get_index_dirty() )
    {
        m_indexBuffer.create_new_buffer(m_mesh->get_indices_size());
        needsRemap = true;
        
        m_mesh->set_index_dirty(false);
    }
    else
    {
        needsRemap = m_indexBuffer.carry_over_buffer();
    }

    if( needsRemap )
    {
        uint8_t* data = m_indexBuffer.map();
        memcpy(data, m_mesh->get_indices().data(), m_mesh->get_indices_size());
        m_indexBuffer.unmap();
    }
}

std::vector<vk::Buffer*> MeshProxy::get_vertex_buffers(uint32_t frameIndex) const
{
    std::vector<vk::Buffer*> retval;
    retval.reserve(m_vertexBuffers.size());

    for( size_t i = 0; i < m_vertexBuffers.size(); i++ )
    {
        retval.push_back(m_vertexBuffers.at(i).get_buffer(frameIndex));
    }

    return retval;
}

vk::Buffer* MeshProxy::get_index_buffer(uint32_t frameIndex) const
{
    return m_indexBuffer.get_buffer(frameIndex);
}

uint32_t MeshProxy::get_index_count() const
{
    return m_indexCount;
}