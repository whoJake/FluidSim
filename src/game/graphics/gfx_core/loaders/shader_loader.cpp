#include "shader_loader.h"
#include <fstream>
#include "system/timer.h"

namespace gfx
{

bool shader_loader::save(const char* filename, const program_def& program)
{
    if( !validate_program(program) )
    {
        GFX_ERROR("Program {} is invalid. Unable to save.", program.name);
        return false;
    }

    std::fstream out_file;
    out_file.open(filename, std::ios::binary | std::ios::trunc | std::ios::out);

    if( !out_file.is_open() )
    {
        GFX_ERROR("Cannot open filename {}", filename);
        return false;
    }

    const u32 magic_number = PROGRAM_MAGIC_NUMBER;

    const u32 string_heap_offset = calculate_string_heap_offset(program);
    const std::vector<u8> string_heap = create_string_heap(program);
    const std::vector<details::descriptor_slot_def> descriptor_slot_heap = create_descriptor_slot_heap(program);
    const std::vector<u8> data_heap = create_data_heap(program);

    details::program_def program_hdr
    {
        .string_heap_offset = string_heap_offset,
        .string_heap_size = u32_cast(string_heap.size()),
        .descriptor_slot_count = u32_cast(descriptor_slot_heap.size()),
        .data_heap_size = u32_cast(data_heap.size()),

        .name_size = u8_cast(program.name.size()),
        .shader_count = u8_cast(program.shaders.size()),
        .pass_count = u8_cast(program.passes.size()),
    };

    std::vector<details::shader_def> shader_hdrs;
    shader_hdrs.reserve(program.shaders.size());

    for( const shader_def& shader : program.shaders )
    {
        details::shader_def shader_hdr
        {
            .entry_point_size = u8_cast(shader.entry_point.size()),
            .data_size = u32_cast(shader.data.size()),
            .stage = shader.stage
        };

        shader_hdrs.push_back(shader_hdr);
    }

    // Passes are a bit different, so we'll serialise this stuff first
    out_file.write((const char*)&magic_number, sizeof(u32));
    out_file.write((const char*)&program_hdr, sizeof(details::program_def));
    out_file.write((const char*)shader_hdrs.data(), sizeof(details::shader_def) * shader_hdrs.size());

    for( const pass_def& pass : program.passes )
    {
        details::pass_def pass_hdr
        {
            .vertex_index = pass.vertex_index,
            .geometry_index = pass.geometry_index,
            .fragment_index = pass.fragment_index,
            .compute_index = pass.compute_index,

            .descriptor_table_count = u8_cast(pass.tables.size()),
            .stage_mask = u16_cast(pass.stage_mask),
            
            .pipeline_state_object = pass.pipeline_state_object,
            .output_formats = pass.output_formats,
        };

        std::vector<details::descriptor_table_def> table_hdrs;
        table_hdrs.reserve(pass.tables.size());

        for( const descriptor_table_def& table : pass.tables )
        {
            details::descriptor_table_def table_hdr
            {
                .buffer_descriptor_slots = u16_cast(table.buffer_slots.size()),
                .image_descriptor_slots = u16_cast(table.image_slots.size()),
            };

            table_hdrs.push_back(table_hdr);
        }

        out_file.write((const char*)&pass_hdr, sizeof(details::pass_def));
        out_file.write((const char*)table_hdrs.data(), sizeof(details::descriptor_table_def) * table_hdrs.size());
    }

    // Write our heaps
    out_file.write((const char*)string_heap.data(), string_heap.size());
    out_file.write((const char*)descriptor_slot_heap.data(), sizeof(details::descriptor_slot_def) * descriptor_slot_heap.size());
    out_file.write((const char*)data_heap.data(), data_heap.size());

    return !out_file.bad();
}

bool shader_loader::load(const char* filename, program_def* out_program)
{
    GFX_ASSERT(out_program, "out_program must not be nullptr.");

    sys::fi_device in_file;
    in_file.open(filename);

    if( !in_file.is_open() )
    {
        GFX_ERROR("Failed to open program file {}", filename);
        return false;
    }

    sys::timer<sys::microseconds> load_timer("Shader load time: {}");

    u32 read_magic{ };
    if( !checked_read(in_file, &read_magic, sizeof(u32)) )
        return false;

    if( read_magic != PROGRAM_MAGIC_NUMBER )
        return false;

    details::program_def read_program{ };
    if( !checked_read(in_file, &read_program, sizeof(details::program_def)) )
        return false;

    std::vector<u8> string_heap;
    string_heap.resize(read_program.string_heap_size);

    std::vector<details::descriptor_slot_def> descriptor_slot_heap;
    descriptor_slot_heap.resize(read_program.descriptor_slot_count);

    std::vector<u8> data_heap;
    data_heap.resize(read_program.data_heap_size);

    in_file.seek_to(read_program.string_heap_offset);
    if( !checked_read(in_file, string_heap.data(), string_heap.size()) )
        return false;

    if( !checked_read(in_file, descriptor_slot_heap.data(), sizeof(details::descriptor_slot_def) * descriptor_slot_heap.size()) )
        return false;

    if( !checked_read(in_file, data_heap.data(), data_heap.size()) )
        return false;

    u64 string_heap_ptr = 0;
    u64 descriptor_heap_ptr = 0;
    u64 data_heap_ptr = 0;

    out_program->name = read_next_string_from_heap(string_heap, string_heap_ptr, read_program.name_size);
    out_program->shaders.reserve(read_program.shader_count);
    out_program->passes.reserve(read_program.pass_count);

    in_file.seek_to(sizeof(u32) + sizeof(details::program_def));
    for( u32 idx = 0; idx < read_program.shader_count; idx++ )
    {
        details::shader_def read_shader;
        if( !checked_read(in_file, &read_shader, sizeof(details::shader_def)) )
            return false;

        shader_def shader
        {
            .entry_point = read_next_string_from_heap(string_heap, string_heap_ptr, read_shader.entry_point_size),
            .stage = read_shader.stage,
            .data = read_next_data_from_heap(data_heap, data_heap_ptr, read_shader.data_size),
        };

        out_program->shaders.emplace_back(std::move(shader));
    }

    for( u32 idx = 0; idx < read_program.pass_count; idx++ )
    {
        details::pass_def read_pass;
        if( !checked_read(in_file, &read_pass, sizeof(details::pass_def)) )
            return false;

        pass_def pass
        {
            .stage_mask = static_cast<gfx::shader_stage_flags>(read_pass.stage_mask),
            .vertex_index = read_pass.vertex_index,
            .geometry_index = read_pass.geometry_index,
            .fragment_index = read_pass.fragment_index,
            .compute_index = read_pass.compute_index,

            .pipeline_state_object = read_pass.pipeline_state_object,
            .output_formats = read_pass.output_formats,
        };

        std::vector<details::descriptor_table_def> read_tables;
        pass.tables.reserve(read_pass.descriptor_table_count);
        read_tables.resize(read_pass.descriptor_table_count);

        if( !checked_read(in_file, read_tables.data(), sizeof(details::descriptor_table_def) * read_pass.descriptor_table_count) )
            return false;

        for( const details::descriptor_table_def& read_table : read_tables )
        {
            std::vector<details::descriptor_slot_def> read_buffer_slots =
                read_next_descriptor_slots(descriptor_slot_heap, descriptor_heap_ptr, read_table.buffer_descriptor_slots);

            std::vector<details::descriptor_slot_def> read_image_slots =
                read_next_descriptor_slots(descriptor_slot_heap, descriptor_heap_ptr, read_table.image_descriptor_slots);

            descriptor_table_def table{ };
            table.buffer_slots.reserve(read_table.buffer_descriptor_slots);
            table.image_slots.reserve(read_table.image_descriptor_slots);

            for( const details::descriptor_slot_def& read_slot : read_buffer_slots )
            {
                descriptor_slot_def slot
                {
                    .name = read_next_string_from_heap(string_heap, string_heap_ptr, read_slot.name_size),
                    .type = read_slot.type,
                    .array_count = read_slot.array_count,
                    .visibility_mask = read_slot.visibility,
                };

                table.buffer_slots.emplace_back(std::move(slot));
            }

            for( const details::descriptor_slot_def& read_slot : read_image_slots )
            {
                descriptor_slot_def slot
                {
                    .name = read_next_string_from_heap(string_heap, string_heap_ptr, read_slot.name_size),
                    .type = read_slot.type,
                    .array_count = read_slot.array_count,
                    .visibility_mask = read_slot.visibility,
                };

                table.image_slots.emplace_back(std::move(slot));
            }

            pass.tables.emplace_back(std::move(table));
        }

        out_program->passes.emplace_back(std::move(pass));
    }

    in_file.close();
    return true;
}

u32 shader_loader::calculate_string_heap_offset(const program_def& program)
{
    u32 out_offset = 0;
    out_offset += u32_cast(sizeof(u32)); // magic number
    out_offset += u32_cast(sizeof(details::program_def));
    out_offset += u32_cast(sizeof(details::shader_def) * program.shaders.size());
    out_offset += u32_cast(sizeof(details::pass_def) * program.passes.size());

    for( const pass_def& pass : program.passes )
    {
        out_offset += u32_cast(sizeof(details::descriptor_table_def) * pass.tables.size());
    }

    return out_offset;
}

std::vector<u8> shader_loader::create_string_heap(const program_def& program)
{
    std::vector<u8> out_heap;

    append_to_string_heap(out_heap, program.name);
    for( const shader_def& shader : program.shaders )
    {
        append_to_string_heap(out_heap, shader.entry_point);
    }

    for( const pass_def& pass : program.passes )
    {
        for( const descriptor_table_def& table : pass.tables )
        {
            for( const descriptor_slot_def& slot : table.buffer_slots )
            {
                append_to_string_heap(out_heap, slot.name);
            }

            for( const descriptor_slot_def& slot : table.image_slots )
            {
                append_to_string_heap(out_heap, slot.name);
            }
        }
    }

    return out_heap;
}

std::vector<details::descriptor_slot_def> shader_loader::create_descriptor_slot_heap(const program_def& program)
{
    std::vector<details::descriptor_slot_def> out_heap;

    for( const pass_def& pass : program.passes )
    {
        for( const descriptor_table_def& table : pass.tables )
        {
            for( const descriptor_slot_def& slot : table.buffer_slots )
            {
                details::descriptor_slot_def slot_hdr
                {
                    .name_size = u8_cast(slot.name.size()),
                    .type = slot.type,
                    .array_count = slot.array_count,
                    .visibility = slot.visibility_mask,
                };

                out_heap.push_back(slot_hdr);
            }

            for( const descriptor_slot_def& slot : table.image_slots )
            {
                details::descriptor_slot_def slot_hdr
                {
                    .name_size = u8_cast(slot.name.size()),
                    .type = slot.type,
                    .array_count = slot.array_count,
                    .visibility = slot.visibility_mask,
                };

                out_heap.push_back(slot_hdr);
            }
        }
    }

    return out_heap;
}

std::vector<u8> shader_loader::create_data_heap(const program_def& program)
{
    std::vector<u8> out_heap;

    for( const shader_def& shader : program.shaders )
    {
        append_to_data_heap(out_heap, shader.data);
    }

    return out_heap;
}

void shader_loader::append_to_string_heap(std::vector<u8>& heap, std::string_view string)
{
    u64 heap_size = heap.size();
    heap.resize(heap_size + string.size());
    memcpy(heap.data() + heap_size, string.data(), string.size());
}

void shader_loader::append_to_data_heap(std::vector<u8>& heap, const std::vector<u8>& data)
{
    u64 heap_size = heap.size();
    heap.resize(heap_size + data.size());
    memcpy(heap.data() + heap_size, data.data(), data.size());
}

std::string shader_loader::read_next_string_from_heap(std::vector<u8>& heap, u64& offset, u64 string_size)
{
    std::string retval((const char*)&heap[offset], string_size);
    offset += string_size;
    return retval;
}

std::vector<u8> shader_loader::read_next_data_from_heap(std::vector<u8>& heap, u64& offset, u64 data_size)
{
    std::vector<u8> retval;
    retval.resize(data_size);
    memcpy(retval.data(), &heap[offset], data_size);
    offset += data_size;
    return retval;
}

std::vector<details::descriptor_slot_def> shader_loader::read_next_descriptor_slots(std::vector<details::descriptor_slot_def>& heap, u64& offset, u64 count)
{
    std::vector<details::descriptor_slot_def> retval;
    retval.resize(count);
    memcpy(retval.data(), &heap[offset], sizeof(details::descriptor_slot_def) * count);
    offset += count;
    return retval;
}

bool shader_loader::checked_read(sys::fi_device& device, void* dst, u64 size)
{
    return device.read((u8*)dst, size) == size;
}

bool shader_loader::validate_program(const program_def& program)
{
    // TODO
    return true;
}

} // gfx