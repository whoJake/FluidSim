#pragma once
#include "hash_string.h"

namespace mtl
{

enum class index_size : u8
{
    size_u8,
    size_u16,
    size_u32,
};

class submesh
{
public:
    submesh();
    submesh(submesh&&) noexcept;
    submesh(const submesh&);
    submesh& operator=(submesh&&) noexcept;
    submesh& operator=(const submesh&);

    ~submesh();

    void add_channel();
    void remove_channel(u64 idx);

    u64 get_vertex_count() const;
    std::vector<glm::vec4>& get_channel(u64 idx);
    void set_vertex_count(u64 count);

    const hash_string& get_material_name() const;
    void set_material_name(const hash_string& name);

    index_size get_index_size() const;
    void set_index_size(index_size size);
private:
    void set_channel_sizes();
private:
    hash_string m_material;
    std::vector<std::vector<glm::vec4>> m_channels;
    std::vector<u8> m_indices;
    u64 m_vertexCount;
    index_size m_indexSize;
};

class mesh
{
public:
    mesh();
    mesh(mesh&&) noexcept;
    mesh(const mesh&);
    mesh& operator=(mesh&&) noexcept;
    mesh& operator=(const mesh&);

    ~mesh();

    u64 add_submesh();
    submesh& get_submesh(u32 idx);
    u64 get_submesh_count() const;

    void set_name(const hash_string& name);
    const hash_string& get_name() const;
private:
    hash_string m_name;
    std::vector<submesh> m_submeshs;
};

} // mtl