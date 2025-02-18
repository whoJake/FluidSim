#pragma once
#include "gfxdefines.h"
#include "types.h"
#include "pipeline_state.h"

#include "dt/hash_string.h"
#include "dt/array.h"
#include "dt/vector.h"
#include "dt/unique_ptr.h"

namespace gfx
{

class program;
class shader;
class pass;
class descriptor_table;
class descriptor_slot_desc;

/*
class program_manager
{
public:
    static const program* find_program(dt::hash_string32 name);
    static void load(const char* path);
private:
    static program_manager& get();
    dt::vector<dt::unique_ptr<program>> m_loadedPrograms;
};
*/

/// Program : A collection of passes that define a rendering sequence. Named after
/// the OpenGL name of a ShaderProgram.
class program
{
public:
    program() = default;
    ~program() = default;

    dt::hash_string32 get_name() const;
    const pass& get_pass(u64 index) const;

    u64 get_pass_count() const;
private:
    dt::hash_string32 m_name;
    dt::array<pass> m_passes;
    dt::array<shader> m_shaders;
};

/// Descriptor Table Description : Describes the layout of a descriptor table. Tables are
/// implicitely ordered with all buffer views being at the start, and all image views being
/// at the end. A descriptor table can be created via a descriptor table desc and then bound
/// onto it via its shader.
/// TODO: This shouldn't have to be ordered like this. It just makes binding with vulkan a bit
/// easier since you can only bind buffers or images at any one time.
/// 
/// In Vulkan, a descriptor_table_desc is roughly equal to a VkDescriptorSetLayout. Multiple
/// descriptor_table_desc therefore make up a VkPipelineLayout.
class descriptor_table_desc
{
public:
    friend class program_manager;

    descriptor_table_desc() = default;
    ~descriptor_table_desc() = default;

    u64 find_buffer_slot(dt::hash_string32 name) const;
    u64 find_image_slot(dt::hash_string32 name) const;

    const dt::array<descriptor_slot_desc>& get_buffer_descriptions() const;
    const dt::array<descriptor_slot_desc>& get_image_descriptions() const;

    GFX_HAS_IMPL(m_pImpl);
private:
    dt::array<descriptor_slot_desc> m_bufferDescs;
    dt::array<descriptor_slot_desc> m_imageDescs;

    void* m_pImpl;
};

/// Pass : A full front of pipeline to back of pipeline program, made up of a combination
/// of shaders. A pass holds the information needed to describe all the resource
/// bindings that will need to happen for it to take effect.
/// 
/// In Vulkan, a pass is roughly equivalent to a combination of VkPipeline and VkPipelineLayout
class pass
{
public:
    friend class program_manager;

    pass() = default;
    ~pass() = default;

    shader_stage_flags get_stage_mask() const;
    u8 get_vertex_shader_index() const;
    u8 get_geometry_shader_index() const;
    u8 get_fragment_shader_index() const;
    u8 get_compute_shader_index() const;

    const descriptor_table_desc& get_descriptor_table(descriptor_table_type type) const;
    const pipeline_state& get_pipeline_state() const;

    GFX_HAS_IMPL(m_pImpl);

    template<typename T>T get_layout_impl()
    {
        return static_cast<T>(m_pLayoutImpl);
    }
    
    template<typename T>T get_layout_impl() const
    {
        return static_cast<T>(m_pLayoutImpl);
    };
private:
    shader_stage_flags m_stageMask;

    u8 m_vertexShaderIndex;
    u8 m_geometryShaderIndex;
    u8 m_fragmentShaderIndex;
    u8 m_computeShaderIndex;

    descriptor_table_desc m_tables[DESCRIPTOR_TABLE_COUNT];
    pipeline_state m_pso;

    void* m_pImpl; // VkPipeline
    void* m_pLayoutImpl; // VkPipelineLayout
};

/// Shader : A singular shader program representing a single shader stage. Multiple shaders
/// make up a pass which will then be bound to the pipeline in order to create a top to bottom
/// effect.
class shader
{
public:
    friend class program_manager;

    shader() = default;
    ~shader() = default;

    void initialise(shader_stage_flag_bits stage);
private:
    dt::hash_string32 m_entryPoint;
    shader_stage_flag_bits m_stage;
    void* m_code;
    u64 m_codeSize;
};

/// Descriptor Slot Description : A slot/binding inside of a descriptor table. Buffer or image
/// views can be bound onto a slot in order for it to be visible to the shader at that location.
class descriptor_slot_desc
{
public:
    descriptor_slot_desc() = default;
    ~descriptor_slot_desc() = default;

    void initialise(dt::hash_string32 name, shader_resource_type type, u32 array_size, u32 slot_size, u32 resource_size, shader_stage_flags visiblility);

    dt::hash_string32 get_name() const;
    shader_resource_type get_resource_type() const;
    u32 get_array_size() const;
    u32 get_slot_size() const;
    u32 get_resource_size() const;
    shader_stage_flags get_visibility() const;
private:
    dt::hash_string32 m_name;
    shader_resource_type m_type;
    u32 m_arraySize;
    u32 m_slotSize;
    u32 m_resourceSize;
    shader_stage_flags m_visibility;
};

/// Descriptor Table : Equal to a descriptor set where the user can bind a set of resources
/// finalise it to write to the underlying API and then bind this descriptor table to a given
/// shader when rendering.
/// TODO: Should this have a concept of being dirty? My thinking is that a descriptor table is
/// flushed at the point it is bound and if its changed then its written to first? Is synchonization
/// something to worry about if I do that?
/// 
/// In Vulkan, a descriptor_table is roughly equivalent to a DescriptorSet and is what binds resources
/// into the pipeline.
class descriptor_table
{
public:
    friend class program_manager;

    descriptor_table() = default;
    ~descriptor_table() = default;

    void initialise(descriptor_table_desc* owner, void* pImpl);

    void set_buffer(dt::hash_string32 name, void* value);
    void set_image(dt::hash_string32 name, void* value);

    const dt::array<void*>& get_buffer_views() const;
    const dt::array<void*>& get_image_views() const;

    /// Write the currently applied views into the underlying
    /// descriptor table.
    void write();

    GFX_HAS_IMPL(m_pImpl);
private:
    descriptor_table_desc* m_desc;

    dt::array<void*> m_bufferViews;
    dt::array<void*> m_imageViews;

    void* m_pImpl;
};

} // gfx