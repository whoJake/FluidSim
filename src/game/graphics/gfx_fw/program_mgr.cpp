#include "program_mgr.h"
#include "gfx_core/shader.h"
#include "gfx_core/Driver.h"
#include "gfx_core/loaders/shader_loader.h"
#include "system/path.h"
#include "system/timer.h"

namespace gfx
{

void program_mgr::initialise(const char* base_directory)
{
    sm_instance = new program_mgr();
    sm_instance->m_baseDirectory = base_directory;
}

void program_mgr::shutdown()
{
    sm_instance->m_cache.destroy();
    for( u64 i = 0; i < sm_instance->m_loadedPrograms.size(); i++ )
    {
        GFX_CALL(destroy_shader_program, sm_instance->m_loadedPrograms[i].get());
    }
    delete sm_instance;
}

const program* program_mgr::find_program(dt::hash_string32 name)
{
    auto it = std::lower_bound(
        sm_instance->m_loadedPrograms.cbegin(),
        sm_instance->m_loadedPrograms.cend(),
        name.get_hash(),
        [](const dt::unique_ptr<program>& prog, u32 hash)
        {
            return prog->get_name().get_hash() < hash;
        });

    if( it == sm_instance->m_loadedPrograms.cend() )
        return nullptr;

    if( it->get()->get_name().get_hash() != name.get_hash() )
        return nullptr;

    return it->get();
}

void program_mgr::load(const char* path)
{
    // Actually load our program so that we can check the name vs what we've got loaded.
    // I don't want to rely on the name of the file to validate this so fuck it these are like
    // a few KB anyway :clueless:
    
    // Replace this with sys::path when i can be bothered to flesh that out
    std::string fullPath = sm_instance->m_baseDirectory + path;

    dt::unique_ptr<program> prog;
    {
        program_def program_def{ };
        if( !shader_loader::load(fullPath.c_str(), &program_def) )
            GFX_ASSERT(false, "Unable to load program path {}", path);

        prog = dt::make_unique<program>(convert_from(program_def));
    }

    GFX_ASSERT(find_program(prog->get_name()) == nullptr, "Program {} at path {} has already been loaded.", prog->get_name().try_get_str(), path);

    sys::timer<sys::microseconds> create_timer("Shader creation time: {}");

    program* pProgram = insert(std::move(prog));
    for( u64 passIdx = 0; passIdx < pProgram->get_pass_count(); passIdx++ )
    {
        pass& pass = pProgram->get_pass(passIdx);
        for( u64 descIdx = 0; descIdx < pass.get_descriptor_table_count(); descIdx++ )
        {
            // Weird interface, have to do this garbage
            // TODO cleanup most likely
            descriptor_table_type type = static_cast<descriptor_table_type>(descIdx);
            descriptor_table_desc* pDesc = pass.get_descriptor_table(type);
            pass.set_descriptor_table(sm_instance->m_cache.get_descriptor_table_desc(std::move(*pDesc)), type);
            delete pDesc;
        }

        pass.set_layout_impl(GFX_CALL(create_shader_pass_layout_impl, &pass));
        pass.set_impl(GFX_CALL(create_shader_pass_impl, pProgram, passIdx));
    }
}

program* program_mgr::insert(dt::unique_ptr<program>&& prog)
{
    auto it = std::lower_bound(
        sm_instance->m_loadedPrograms.cbegin(),
        sm_instance->m_loadedPrograms.cend(),
        prog->get_name().get_hash(),
        [](const dt::unique_ptr<program>& prog, u32 hash)
        {
            return prog->get_name().get_hash() < hash;
        });
    u64 index = sm_instance->m_loadedPrograms.index_of(it);
    program* retval = prog.get();
    sm_instance->m_loadedPrograms.insert(index, std::move(prog));
    return retval;
}

program program_mgr::convert_from(const program_def& program_def)
{
    program retval{ };
    retval.m_name = dt::hash_string32(program_def.name);
    retval.m_shaders.initialise(program_def.shaders.size());
    retval.m_passes.initialise(program_def.passes.size());

    for( u64 idx = 0; idx < program_def.shaders.size(); idx++ )
    {
        const shader_def& shader_def = program_def.shaders[idx];
        shader& shader = retval.m_shaders[idx];

        shader.m_code.initialise(shader_def.data.size() / sizeof(u32), false);
        memcpy(shader.m_code.data(), shader_def.data.data(), shader.m_code.size() * sizeof(u32));

        shader.m_entryPoint = dt::hash_string32(shader_def.entry_point);
        shader.m_stage = shader_def.stage;
    }

    for( u64 idx = 0; idx < program_def.passes.size(); idx++ )
    {
        const pass_def& pass_def = program_def.passes[idx];
        pass& pass = retval.m_passes[idx];

        pass.m_stageMask = pass_def.stage_mask;
        pass.m_vertexShaderIndex = pass_def.vertex_index;
        pass.m_geometryShaderIndex = pass_def.geometry_index;
        pass.m_fragmentShaderIndex = pass_def.fragment_index;
        pass.m_computeShaderIndex = pass_def.compute_index;

        pass.m_tableCount = u32_cast(pass_def.tables.size());
        pass.m_pso = pass_def.pipeline_state_object;
        pass.m_outputs = pass_def.output_formats;

        for( u32 tableIdx = 0; tableIdx < pass.m_tableCount; tableIdx++ )
        {
            const descriptor_table_def& table_def = pass_def.tables[tableIdx];
            descriptor_table_desc* table = new descriptor_table_desc();

            dt::vector<descriptor_slot_desc> buffer_slots;
            buffer_slots.reserve(table_def.buffer_slots.size());

            dt::vector<descriptor_slot_desc> image_slots;
            image_slots.reserve(table_def.image_slots.size());

            for( const descriptor_slot_def& slot_def : table_def.buffer_slots )
            {
                descriptor_slot_desc slot;
                slot.initialise(dt::hash_string32(slot_def.name), slot_def.type, slot_def.array_count, 0, 0, slot_def.visibility_mask);
                buffer_slots.emplace_back(std::move(slot));
            }

            for( const descriptor_slot_def& slot_def : table_def.image_slots )
            {
                descriptor_slot_desc slot;
                slot.initialise(dt::hash_string32(slot_def.name), slot_def.type, slot_def.array_count, 0, 0, slot_def.visibility_mask);
                image_slots.emplace_back(std::move(slot));
            }

            table->initialise(buffer_slots, image_slots);

            pass.m_tables[idx] = table;
        }
    }

    return retval;
}

descriptor_table_desc* descriptor_cache::get_descriptor_table_desc(descriptor_table_desc&& desc)
{
    u64 desc_hash = desc.calculate_hash();

    auto it = std::lower_bound(
        m_tableDescHashes.cbegin(),
        m_tableDescHashes.cend(),
        desc_hash);

    bool found = !(it == m_tableDescHashes.cend() || *it != desc_hash);
    u64 index = m_tableDescHashes.index_of(it);

    if( !found )
    {
        // Insert it into our table_descs and give it an impl.
        GFX_ASSERT(desc.get_impl<void*>() == nullptr, "Passing in descriptor_table_desc that is new to the descriptor_cache but already has an impl. Is this behaviour intended?");
        desc.set_impl(GFX_CALL(create_descriptor_table_desc_impl, &desc));
        m_tableDescHashes.insert(index, desc_hash);
        m_tableDescs.insert(index, std::move(desc));
        return &m_tableDescs[index];
    }
    else
    {
        return &m_tableDescs[index];
    }
}

void descriptor_cache::destroy()
{
    for( descriptor_table_desc& desc : m_tableDescs )
    {
        GFX_CALL(destroy_descriptor_table_desc, &desc);
        desc.set_impl(nullptr);
    }
}

} // gfx