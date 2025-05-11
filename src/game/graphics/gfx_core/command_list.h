#pragma once

#include "pipeline_state.h"
#include "buffer.h"
#include "fence.h"
#include "texture.h"
#include "dependency.h"
#include "descriptor_pool.h"
#include "shader.h"

namespace gfx
{

enum class command_list_type
{
    graphics = 0,
    compute,
    transfer,
    present,

    count,
};

class command_list
{
public:
    ~command_list() = default;

    void init(void* pImpl, bool is_secondary = false);
    void reset(bool keep_dependencies);

    void begin();
    void end();

    void submit(fence* fence = nullptr);
    const command_list_type& get_type() const;
    
    // Commands
    // void texture_memory_barrier(texture* texture, texture_layout dst_layout, memory_access_mask dst_access);
    void texture_memory_barrier(texture* texture, texture_layout dst_layout, pipeline_stage_flag_bits src_stage = PIPELINE_STAGE_BOTTOM_OF_PIPE, pipeline_stage_flag_bits dst_stage = PIPELINE_STAGE_TOP_OF_PIPE);
    // Pipeline barriers here !

    void bind_program(program* prog, u64 passIdx);

    void execute_command_lists(command_list** pLists, u32 count);

    void add_wait_dependency(dependency* dep);
    void remove_wait_dependency(dependency* dep);
    const std::vector<dependency*>& get_wait_dependencies() const;

    void set_signal_dependency(dependency* dep);
    const dependency* get_signal_dependency() const;

    bool is_secondary() const;

    GFX_HAS_IMPL(m_pImpl);
protected:
    command_list(command_list_type type);

    pipeline_state m_pso;
    command_list_type m_type;

    // Make this a sorted array
    std::vector<dependency*> m_waitDependencies;
    dependency* m_signalDependency;

    bool m_isActive{ false };
    bool m_isSecondary{ false };
private:
    u8 m_unused[2];
    void* m_pImpl;
};

class transfer_command_list : public command_list
{
public:
    transfer_command_list();
    ~transfer_command_list() = default;

    // Commands
    void copy_to_texture(texture* src, texture* dst);
    void copy_to_texture(buffer* src, texture* dst);
    void copy_buffer(buffer* src, buffer* dst);

    // This doesn't work on transfer, its just shared between graphics and compute.. how..
    void bind_descriptor_tables(pass* pass, descriptor_table** pTables, u32 table_count, descriptor_table_type type);
protected:
    transfer_command_list(command_list_type override_type);
};

class graphics_command_list : public transfer_command_list
{
public:
    graphics_command_list();
    ~graphics_command_list() = default;
    
    // Commands
    void begin_rendering(const std::vector<texture_attachment>& color_attachment, texture_attachment* depth_attachment);
    void end_rendering();

    void draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0);
    void draw_indexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0, u32 vertex_offset = 0, u32 first_instance = 0);

    void bind_vertex_buffers(buffer** pBuffers, u32 buffer_count, u32 first_vertex_index = 0);
    void bind_index_buffer(buffer* buffer, index_buffer_type index_type = index_buffer_type::INDEX_TYPE_U16);

    void set_viewport(f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth);
    void set_scissor(u32 x, u32 y, u32 width, u32 height);
};

class compute_command_list : public transfer_command_list
{
public:
    compute_command_list();
    ~compute_command_list() = default;

    // Commands
};

class present_command_list : public command_list
{
public:
    present_command_list();
    ~present_command_list() = default;

    // Commands
};

} // gfx