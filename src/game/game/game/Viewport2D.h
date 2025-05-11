#pragma once

class Viewport2D
{
public:
    Viewport2D() = default;
    Viewport2D(glm::vec2 screen_extent, glm::vec2 view_position, glm::vec2 view_size);
    ~Viewport2D() = default;

    DEFAULT_MOVE(Viewport2D);
    DEFAULT_COPY(Viewport2D);
    
    glm::vec2 get_screen_extent() const;
    void set_screen_extent(glm::vec2 extent);

    glm::vec2 get_view_position() const;
    void set_view_position(glm::vec2 position);

    glm::vec2 get_view_extent() const;
    void set_view_extent(glm::vec2 extent);

    bool is_dirty() const;

    void update_matrices();
private:
    glm::vec2 m_screenExtent;
    glm::vec2 m_viewPosition;
    glm::vec2 m_viewExtent;
    u64 m_dirty : 1;
    u64 m_pad : 63;
};