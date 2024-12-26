#include "pipeline_state.h"

namespace gfx
{

void pipeline_state::reset()
{
    if( !is_dirty() )
    {
        return;
    }

    m_vertexInputState = { };
    m_inputAssemblyState = { };
    m_tessellationState = { };
    m_rasterizationState = { };
    m_multisampleState = { };
    m_depthStencilState = { };
    m_outputBlendStates = { };
    
    clear_dirty();
}

void pipeline_state::clear_dirty()
{
    m_dirty = false;
}

bool pipeline_state::is_dirty() const
{
    return m_dirty;
}

void pipeline_state::set_vertex_input_state(const vertex_input_state& state)
{
    if( m_vertexInputState != state )
    {
        m_vertexInputState = state;
        m_dirty = true;
    }
}

void pipeline_state::set_input_assembly_state(const input_assembly_state& state)
{
    if( m_inputAssemblyState != state )
    {
        m_inputAssemblyState = state;
        m_dirty = true;
    }
}

void pipeline_state::set_tessellation_state(const tessellation_state& state)
{
    if( m_tessellationState != state )
    {
        m_tessellationState = state;
        m_dirty = true;
    }
}

void pipeline_state::set_rasterization_state(const rasterization_state& state)
{
    if( m_rasterizationState != state )
    {
        m_rasterizationState = state;
        m_dirty = true;
    }
}

void pipeline_state::set_multisample_state(const multisample_state& state)
{
    if( m_multisampleState != state )
    {
        m_multisampleState = state;
        m_dirty = true;
    }
}

void pipeline_state::set_depth_stencil_state(const depth_stencil_state& state)
{
    if( m_depthStencilState != state )
    {
        m_depthStencilState = state;
        m_dirty = true;
    }
}

void pipeline_state::set_output_blend_states(const output_blend_states& state)
{
    if( m_outputBlendStates != state )
    {
        m_outputBlendStates = state;
        m_dirty = true;
    }
}

const vertex_input_state& pipeline_state::get_vertex_input_state() const
{
    return m_vertexInputState;
}

const input_assembly_state& pipeline_state::get_input_assembly_state() const
{
    return m_inputAssemblyState;
}

const tessellation_state& pipeline_state::get_tessellation_state() const
{
    return m_tessellationState;
}

const rasterization_state& pipeline_state::get_rasterization_state() const
{
    return m_rasterizationState;
}

const multisample_state& pipeline_state::get_multisample_state() const
{
    return m_multisampleState;
}

const depth_stencil_state& pipeline_state::get_depth_stencil_state() const
{
    return m_depthStencilState;
}

const output_blend_states& pipeline_state::get_output_blend_states() const
{
    return m_outputBlendStates;
}

bool operator!=(const vertex_input_state& lhs, const vertex_input_state& rhs)
{
    if( lhs.channel_count != rhs.channel_count )
        return true;

    for( u32 i = 0; i < lhs.channel_count; i++ )
    {
        if( lhs.descriptions[i] != rhs.descriptions[i] )
            return true;
    }

    return false;
}

bool operator!=(const input_assembly_state& lhs, const input_assembly_state& rhs)
{
    return std::tie(lhs.topology, lhs.allow_restart)
        != std::tie(rhs.topology, rhs.allow_restart);
}

bool operator!=(const tessellation_state& lhs, const tessellation_state& rhs)
{
    return lhs.patch_control_points
        != rhs.patch_control_points;
}

bool operator!=(const rasterization_state& lhs, const rasterization_state& rhs)
{
    return std::tie(lhs.polygon_mode, lhs.cull_mode, lhs.front_face_mode, lhs.enable_depth_bias, lhs.depth_bias_mode, lhs.line_width, lhs.enable_depth_clamp, lhs.enable_rasterizer_discard)
        != std::tie(rhs.polygon_mode, rhs.cull_mode, rhs.front_face_mode, rhs.enable_depth_bias, rhs.depth_bias_mode, rhs.line_width, rhs.enable_depth_bias, rhs.enable_rasterizer_discard);
}

bool operator!=(const multisample_state& lhs, const multisample_state& rhs)
{
    return std::tie(lhs.sample_count, lhs.min_sample_shading, lhs.sample_mask, lhs.enable_alpha_to_coverage, lhs.enable_alpha_to_one, lhs.enable_sample_shading)
        != std::tie(rhs.sample_count, rhs.min_sample_shading, rhs.sample_mask, rhs.enable_alpha_to_coverage, rhs.enable_alpha_to_one, rhs.enable_sample_shading);
}

bool operator!=(const depth_stencil_state& lhs, const depth_stencil_state& rhs)
{
    return std::tie(lhs.depth_compare, lhs.front_stencil, lhs.back_stencil, lhs.enable_depth_bounds_test, lhs.enable_stencil_test, lhs.enable_depth_test, lhs.write_depth)
        != std::tie(rhs.depth_compare, rhs.front_stencil, rhs.back_stencil, rhs.enable_depth_bounds_test, rhs.enable_stencil_test, rhs.enable_depth_test, rhs.write_depth);
}

bool operator!=(const output_blend_states& lhs, const output_blend_states& rhs)
{
    if( std::tie(lhs.enable_blend, lhs.logic_op, lhs.state_count)
        != std::tie(rhs.enable_blend, rhs.logic_op, rhs.state_count) )
        return true;

    for( u32 i = 0; i < lhs.state_count; i++ )
    {
        if( lhs.blend_states[i] != rhs.blend_states[i] )
            return true;
    }

    return false;
}

bool operator!=(const vertex_input_attributes& lhs, const vertex_input_attributes& rhs)
{
    return std::tie(lhs.format, lhs.offset)
        != std::tie(rhs.format, rhs.offset);
}

bool operator!=(const vertex_input_description& lhs, const vertex_input_description& rhs)
{
    if( std::tie(lhs.stride, lhs.input_rate, lhs.attribute_count)
        != std::tie(rhs.stride, rhs.input_rate, rhs.attribute_count) )
        return true;

    for( u32 i = 0; i < lhs.attribute_count; i++ )
    {
        if( lhs.attributes[i] != rhs.attributes[i] )
            return true;
    }

    return false;
}

bool operator==(const depth_bias_mode& lhs, const depth_bias_mode& rhs)
{
    return std::tie(lhs.constant_factor, lhs.slope_factor)
        == std::tie(rhs.constant_factor, rhs.slope_factor);
}

bool operator==(const stencil_operation_state& lhs, const stencil_operation_state& rhs)
{
    return std::tie(lhs.fail, lhs.pass, lhs.depth_fail, lhs.compare)
        == std::tie(rhs.fail, rhs.pass, rhs.depth_fail, rhs.compare);
}

bool operator!=(const color_blend_state& lhs, const color_blend_state& rhs)
{
    return std::tie(lhs.src_color_factor, lhs.dst_color_factor, lhs.color_operation, lhs.src_alpha_factor, lhs.dst_alpha_factor, lhs.alpha_operation)
        != std::tie(rhs.src_color_factor, rhs.dst_color_factor, rhs.color_operation, rhs.src_alpha_factor, rhs.dst_alpha_factor, rhs.alpha_operation);
}


} // gfx