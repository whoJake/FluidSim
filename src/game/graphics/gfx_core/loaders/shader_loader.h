#pragma once

#include "system/magic_numbers.h"
#include "system/device.h"
#include "../types.h"
#include "../pipeline_state.h"
#include <string>

namespace gfx
{

constexpr u32 PROGRAM_MAGIC_NUMBER = MAKE_MAGIC_NUMBER('F', 'X', 'C', 'P');

struct program_def;
struct pass_def;
struct shader_def;
struct descriptor_table_def;
struct descriptor_slot_def;

namespace details
{
struct descriptor_slot_def;
} // details

class shader_loader
{
public:
    shader_loader() = delete;
    ~shader_loader() = delete;

    static bool save(const char* filename, const program_def& program);
    static bool load(const char* filename, program_def* out_program);
private:
    static bool validate_program(const program_def& program);

    // Saving
    static u32 calculate_string_heap_offset(const program_def& program);

    static std::vector<u8> create_string_heap(const program_def& program);
    static std::vector<details::descriptor_slot_def> create_descriptor_slot_heap(const program_def& program);
    static std::vector<u8> create_data_heap(const program_def& program);

    static void append_to_string_heap(std::vector<u8>& heap, std::string_view string);
    static void append_to_data_heap(std::vector<u8>& heap, const std::vector<u8>& data);

    // Loading
    static std::string read_next_string_from_heap(std::vector<u8>& heap, u64& offset, u64 string_size);
    static std::vector<u8> read_next_data_from_heap(std::vector<u8>& heap, u64& offset, u64 data_size);
    static std::vector<details::descriptor_slot_def> read_next_descriptor_slots(std::vector<details::descriptor_slot_def>& heap, u64& offset, u64 count);

    static bool checked_read(sys::fi_device& device, void* dst, u64 size);
};

struct program_def
{
    std::string name;
    std::vector<shader_def> shaders;
    std::vector<pass_def> passes;
};

struct shader_def
{
    std::string entry_point;
    shader_stage_flag_bits stage;
    std::vector<u8> data;
};

struct pass_def
{
    shader_stage_flags stage_mask;
    u8 vertex_index;
    u8 geometry_index;
    u8 fragment_index;
    u8 compute_index;
    std::vector<descriptor_table_def> tables;
    pipeline_state pipeline_state_object;
    shader_pass_outputs output_formats;
};

struct descriptor_table_def
{
    std::vector<descriptor_slot_def> buffer_slots;
    std::vector<descriptor_slot_def> image_slots;
};

struct descriptor_slot_def
{
    std::string name;
    shader_resource_type type;
    u32 array_count;
    shader_stage_flags visibility_mask;
};

namespace details
{

/// High level description of how this is laid out in memory
/// 
/// FILE
/// | MAGIC_NUMBER | PROGRAM_DEF | SHADER_DEF x N | PASS x N | STRING_HEAP | DESCRIPTOR_SLOT_HEAP | DATA_HEAP |
/// 
/// PASS
/// | PASS_DEF | DESCRIPTOR_TABLE_DEF x N |

struct program_def;
struct pass_def;
struct shader_def;
struct descriptor_table_def;
struct descriptor_slot_def;

struct program_def
{
    u32 string_heap_offset;
    u32 string_heap_size;
    u32 descriptor_slot_count;
    u32 data_heap_size;

    u8 name_size;
    u8 shader_count;
    u8 pass_count;
    u8 pad{ };
};

struct shader_def
{
    u32 entry_point_size : 8;
    u32 data_size : 24;
    shader_stage_flag_bits stage;
};

struct pass_def
{
    u8 vertex_index;
    u8 geometry_index;
    u8 fragment_index;
    u8 compute_index;
    
    u8 descriptor_table_count;
    u8 pad{ };
    u16 stage_mask;

    pipeline_state pipeline_state_object;
    shader_pass_outputs output_formats;
};

struct descriptor_table_def
{
    u16 buffer_descriptor_slots;
    u16 image_descriptor_slots;
};

struct descriptor_slot_def
{
    u8 name_size;
    u8 pad[3]{ };
    shader_resource_type type;
    u32 array_count;
    shader_stage_flags visibility;
};

} // details

} // gfx