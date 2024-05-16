#pragma once

#include "scene/gameplay/Mesh.h"
#include "rendering/ContextBackedBuffer.h"

class MeshProxy
{
public:
    MeshProxy(vk::RenderContext* context, Mesh* mesh);
    ~MeshProxy() = default;

    void sync();

    std::vector<vk::Buffer*> get_vertex_buffers(uint32_t frameIndex) const;
    vk::Buffer* get_index_buffer(uint32_t frameIndex) const;

    uint32_t get_index_count() const;
private:
    Mesh* m_mesh;
    uint32_t m_indexCount;
    std::vector<vk::ContextBackedBuffer> m_vertexBuffers;
    vk::ContextBackedBuffer m_indexBuffer;
};