#pragma once
#include "types.h"
#include "gfxdefines.h"

#if 1
#define GFX_PSO_ENABLE_CHECKS 1
#endif

namespace gfx
{

// Ordered to match VkVertexInputRate
enum class vertex_input_rate
{
    vertex = 0,
    instance,

    count,
};

struct vertex_input_attributes
{
    format format{ format::UNDEFINED };
    u32 offset;
};

struct vertex_input_description
{
    u32 stride;
    vertex_input_rate input_rate;

    u32 attribute_count{ 0 };
    vertex_input_attributes attributes[GFX_MAX_VERTEX_ATTRIBUTES_PER_CHANNEL];
};

// Ordered to match VkFrontFace
enum class front_face_mode
{
    counter_clockwise = 0,
    clockwise,

    count,
};

// Ordered to match VkPrimitiveTopology
enum class topology_mode
{
    point_list = 0,
    line_list,
    line_strip,
    triangle_list,
    triangle_strip,
    triangle_fan,
    line_list_adj,
    line_strip_adj,
    triangle_list_adj,
    triangle_strip_adj,
    patch_list,

    count
};

// Ordered to match VkSampleCountFlagBits
enum class sample_count_flag_bits : u32
{
    count_1 = 1 << 0,
    count_2 = 1 << 1,
    count_4 = 1 << 2,
    count_8 = 1 << 3,
    count_16 = 1 << 4,
    count_32 = 1 << 5,
    count_64 = 1 << 6,

    count,
};
using sample_count_flags = std::underlying_type_t<sample_count_flag_bits>;

enum class polygon_mode
{
    fill = 0,
    line,
    point,

    count,
};

enum class cull_mode
{
    none = 0,
    front,
    back,
    front_and_back,

    count,
};

struct depth_bias_mode
{
    f32 constant_factor{ 1.f };
    f32 slope_factor{ 1.f };
};

// Ordered to match VkStencilOp
enum class stencil_operation
{
    keep = 0,
    zero,
    replace,
    increment_and_clamp,
    decrement_and_clamp,
    invert,
    increment_and_wrap,
    decrement_and_wrap,

    count,
};

// Ordered to match VkLogicOp
enum class logic_operation
{
    clear = 0,
    and_,
    and_reverse,
    copy,
    and_inverted,
    no_op,
    xor_,
    or_,
    nor_,
    equivalent,
    invert,
    or_reverse,
    copy_inverted,
    or_inverted,
    nand_,
    set,

    count,
};

// Ordered to match VkCompareOp
enum class compare_operation
{
    never = 0,
    less,
    equal,
    less_or_equal,
    greater,
    not_equal,
    greater_or_equal,
    always,

    count,
};

// Ordered to match VkBlendFactor
enum class blend_factor
{
    zero = 0,
    one,
    src_color,
    one_minus_src_color,
    dst_color,
    one_minus_dst_color,
    src_alpha,
    one_minus_src_alpha,
    dst_alpha,
    one_minus_dst_alpha,
    constant_color,
    one_minus_constant_color,
    constant_alpha,
    one_minus_constant_alpha,
    src_alpha_saturate,
    src1_color,
    one_minus_src1_color,
    src1_alplha,
    one_minus_src1_alpha,

    count,
};

// Ordered to match VkBlendOp
enum class blend_operation
{
    add = 0,
    subtract,
    reverse_subtract,
    min,
    max,

    count,
};

struct vertex_input_state
{
    u32 channel_count{ 0 };
    vertex_input_description descriptions[GFX_MAX_VERTEX_INPUT_CHANNELS]{ };
};

struct input_assembly_state
{
    topology_mode topology{ topology_mode::triangle_list };
    bool allow_restart{ false };
    u8 unused[3];
};

struct tessellation_state
{
    u32 patch_control_points{ 0 };
};

struct rasterization_state
{
    polygon_mode polygon_mode{ polygon_mode::fill };
    cull_mode cull_mode{ cull_mode::none };
    front_face_mode front_face_mode{ front_face_mode::counter_clockwise };
    depth_bias_mode depth_bias_mode{ };
    f32 line_width{ 1.f };
    bool enable_depth_clamp{ false };
    bool enable_rasterizer_discard{ false };
    bool enable_depth_bias{ false };
    u8 unused;
};

struct multisample_state
{
    sample_count_flags sample_count;
    f32 min_sample_shading;
    u32 sample_mask;
    bool enable_alpha_to_coverage;
    bool enable_alpha_to_one;
    bool enable_sample_shading;
    u8 unused;
};

struct stencil_operation_state
{
    stencil_operation fail{ stencil_operation::replace };
    stencil_operation pass{ stencil_operation::replace };
    stencil_operation depth_fail{ stencil_operation::replace };
    compare_operation compare{ compare_operation::never };
};

struct depth_stencil_state
{
    compare_operation depth_compare{ compare_operation::less_or_equal };
    stencil_operation_state front_stencil{ };
    stencil_operation_state back_stencil{ };
    bool enable_depth_bounds_test{ false };
    bool enable_stencil_test{ false };
    bool enable_depth_test{ true };
    bool write_depth{ true };
};

struct color_blend_state
{
    blend_factor src_color_factor{ blend_factor::one };
    blend_factor dst_color_factor{ blend_factor::zero };
    blend_operation color_operation{ blend_operation::add };

    blend_factor src_alpha_factor{ blend_factor::one };
    blend_factor dst_alpha_factor{ blend_factor::zero };
    blend_operation alpha_operation{ blend_operation::add };

    bool enable_blend{ false };
    u8 unused[3];
    // Channel mask?
};

struct output_blend_states
{
    bool enable_blend{ false };
    logic_operation logic_op{ logic_operation::clear };
    u32 state_count{ 0 };
    color_blend_state blend_states[GFX_MAX_OUTPUT_ATTACHMENTS]{ };
};

class pipeline_state
{
public:
    pipeline_state() = default;
    ~pipeline_state() = default;

    DEFAULT_MOVE(pipeline_state);
    DEFAULT_COPY(pipeline_state);

    void reset();
    void clear_dirty();
    bool is_dirty() const;

    // Base setters
    void set_vertex_input_state(const vertex_input_state& state);
    void set_input_assembly_state(const input_assembly_state& state);
    void set_tessellation_state(const tessellation_state& state);
    // void set_viewport_state(const viewport_state& state);
    void set_rasterization_state(const rasterization_state& state);
    void set_multisample_state(const multisample_state& state);
    void set_depth_stencil_state(const depth_stencil_state& state);
    void set_output_blend_states(const output_blend_states& state);

    // Base getters
    const vertex_input_state& get_vertex_input_state() const;
    const input_assembly_state& get_input_assembly_state() const;
    const tessellation_state& get_tessellation_state() const;
    // const viewport_state& get_viewport_state() const;
    const rasterization_state& get_rasterization_state() const;
    const multisample_state& get_multisample_state() const;
    const depth_stencil_state& get_depth_stencil_state() const;
    const output_blend_states& get_output_blend_states() const;
private:
    vertex_input_state m_vertexInputState{ };
    input_assembly_state m_inputAssemblyState{ };
    tessellation_state m_tessellationState{ };
    // viewport_state
    rasterization_state m_rasterizationState{ };
    multisample_state m_multisampleState{ };
    depth_stencil_state m_depthStencilState{ };
    output_blend_states m_outputBlendStates{ };

    bool m_dirty{ false };
};

bool operator!=(const vertex_input_state& lhs, const vertex_input_state& rhs);
bool operator!=(const input_assembly_state& lhs, const input_assembly_state& rhs);
bool operator!=(const tessellation_state& lhs, const tessellation_state& rhs);
bool operator!=(const rasterization_state& lhs, const rasterization_state& rhs);
bool operator!=(const multisample_state& lhs, const multisample_state& rhs);
bool operator!=(const depth_stencil_state& lhs, const depth_stencil_state& rhs);
bool operator!=(const output_blend_states& lhs, const output_blend_states& rhs);

bool operator!=(const vertex_input_attributes& lhs, const vertex_input_attributes& rhs);
bool operator!=(const vertex_input_description& lhs, const vertex_input_description& rhs);
bool operator==(const depth_bias_mode& lhs, const depth_bias_mode& rhs);
bool operator==(const stencil_operation_state& lhs, const stencil_operation_state& rhs);
bool operator!=(const color_blend_state & lhs, const color_blend_state & rhs);

} // gfx