#pragma once

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 colour;

    bool operator==(const Vertex& other)
    {
        return position == other.position
            && normal == other.normal
            && colour == other.colour;
    }

    bool operator!=(const Vertex& other)
    {
        return !(*this == other);
    }
};

// template<class V = Vertex, class T = uint16_t>
class Mesh
{
    /*
    static_assert(std::is_same_v<T, uint16_t>
               || std::is_same_v<T, uint32_t>,
        "Indices only supports types <uint16_t, uint32_t>");
        */
    typedef Vertex V;
    typedef uint16_t T;
public:
    Mesh(const std::vector<std::vector<V>>& vertices = { },
         const std::vector<T>& indices = { });

    Mesh(uint32_t vertexBufferCount = 1,
         const std::vector<T>& indices = { });

    ~Mesh() = default;
    Mesh(Mesh&&) = default;
    Mesh(const Mesh&) = default;
    Mesh& operator=(Mesh&&) = default;
    Mesh& operator=(const Mesh&) = default;

    void set_vertex(size_t i, const V& vertex, uint32_t index);
    void set_index(size_t i, const T& index);
    void set_vertices(const std::vector<V>& vertices, uint32_t index);
    void set_indices(const std::vector<T>& indices);

    const std::vector<V>& get_vertices(uint32_t index) const;
    const std::vector<T>& get_indices() const;

    uint32_t get_vertex_buffer_count() const;

    size_t get_vertex_count() const;
    size_t get_index_count() const;

    size_t get_vertex_stride(uint32_t index) const;
    size_t get_vertices_size(uint32_t index) const;
    size_t get_indices_size() const;

    void set_vertex_dirty(uint32_t index, bool value = true);
    void set_index_dirty(bool value = true);

    bool get_vertex_dirty(uint32_t index) const;
    bool get_index_dirty() const;

    void recalculate_normals();
private:
    void resize_vertex_buffers(uint32_t size);
private:
    std::vector<std::vector<V>> m_vertices;
    std::vector<T> m_indices;

    std::vector<bool> m_vertexDirty;
    bool m_indexDirty;

    uint32_t m_vertexCount{ 0 };
};
