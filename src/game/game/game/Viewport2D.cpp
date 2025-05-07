#include "Viewport2D.h"

Viewport2D::Viewport2D(glm::vec2 screen_extent, glm::vec2 view_position, glm::vec2 view_extent) :
    m_screenExtent(screen_extent),
    m_viewPosition(view_position),
    m_viewExtent(view_extent),
    m_dirty(1)
{
    update_matrices();
}

glm::vec2 Viewport2D::get_screen_extent() const
{
    return m_screenExtent;
}

void Viewport2D::set_screen_extent(glm::vec2 extent)
{
    m_dirty = 1;
    m_screenExtent = extent;
}

glm::vec2 Viewport2D::get_view_position() const
{
    return m_viewPosition;
}

void Viewport2D::set_view_position(glm::vec2 position)
{
    m_dirty = 1;
    m_viewPosition = position;
}

glm::vec2 Viewport2D::get_view_extent() const
{
    return m_viewExtent;
}

void Viewport2D::set_view_extent(glm::vec2 extent)
{
    m_dirty = 1;
    m_viewExtent = extent;
}

bool Viewport2D::is_dirty() const
{
    return m_dirty;
}

void Viewport2D::update_matrices()
{
    if( !is_dirty() )
        return;

    m_dirty = 0;
}