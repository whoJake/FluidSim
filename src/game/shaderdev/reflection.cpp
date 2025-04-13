#include "reflection.h"
#include "dt/unique_ptr.h"
#include "shadev_channels.h"

#pragma region read_resource_X

static void read_resource_vec_size(const spirv_cross::Compiler& compiler,
                                   const spirv_cross::Resource& resource,
                                   shader_resource* shader_resource)
{
    const spirv_cross::SPIRType& type = compiler.get_type_from_variable(resource.id);
    shader_resource->vecSize = type.vecsize;
    shader_resource->columns = type.columns;
}

static void read_resource_array_size(const spirv_cross::Compiler& compiler,
                                     const spirv_cross::Resource& resource,
                                     shader_resource* shader_resource)
{
    const spirv_cross::SPIRType& type = compiler.get_type_from_variable(resource.id);
    shader_resource->arraySize = type.array.size()
        ? type.array[0]
        : 1;
}

static void read_resource_struct_size(const spirv_cross::Compiler& compiler,
                                      const spirv_cross::Resource& resource,
                                      shader_resource* shader_resource)
{
    const spirv_cross::SPIRType& type = compiler.get_type_from_variable(resource.id);
    shader_resource->stride = u32_cast(compiler.get_declared_struct_size(type));
}

static void read_resource_size(const spirv_cross::Compiler& compiler,
                               const spirv_cross::SPIRConstant& constant,
                               shader_resource* shader_resource)
{
    const spirv_cross::SPIRType& type = compiler.get_type(constant.constant_type);

    switch( type.basetype )
    {
    case spirv_cross::SPIRType::BaseType::Boolean:
    case spirv_cross::SPIRType::BaseType::Char:
    case spirv_cross::SPIRType::BaseType::Int:
    case spirv_cross::SPIRType::BaseType::UInt:
    case spirv_cross::SPIRType::BaseType::Float:
        shader_resource->stride = 4;
        break;
    case spirv_cross::SPIRType::BaseType::Int64:
    case spirv_cross::SPIRType::BaseType::UInt64:
    case spirv_cross::SPIRType::BaseType::Double:
        shader_resource->stride = 8;
        break;
    default:
        shader_resource->stride = 0;
        break;
    }
}

#pragma endregion read_resource_X

#pragma region read_shader_resource<>

template<gfx::shader_resource_type T>
static bool read_shader_resource(const spirv_cross::Compiler& compiler,
                                 const spirv_cross::ShaderResources& spirv_resources,
                                 dt::vector<shader_resource>* out_resources)
{
    static_assert(false, "Resource type of template has not been implemented.");
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_INPUT>(const spirv_cross::Compiler& compiler,
                                                            const spirv_cross::ShaderResources& spirv_resources,
                                                            dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.stage_inputs )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_INPUT;

        ret_resource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);

        read_resource_vec_size(compiler, resource, &ret_resource);
        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }

    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_INPUT_ATTACHMENT>(const spirv_cross::Compiler& compiler,
                                                                        const spirv_cross::ShaderResources& spirv_resources,
                                                                        dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.subpass_inputs )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_INPUT_ATTACHMENT;

        ret_resource.inputAttachmentIndex = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);
        ret_resource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        ret_resource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }

    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_OUTPUT>(const spirv_cross::Compiler& compiler,
                                                              const spirv_cross::ShaderResources& spirv_resources,
                                                              dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.stage_outputs )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_OUTPUT;

        ret_resource.location = compiler.get_decoration(resource.id, spv::DecorationLocation);

        read_resource_vec_size(compiler, resource, &ret_resource);
        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }

    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_IMAGE>(const spirv_cross::Compiler& compiler,
                                                             const spirv_cross::ShaderResources& spirv_resources,
                                                             dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.separate_images )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_IMAGE;

        ret_resource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        ret_resource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }
    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_IMAGE_SAMPLER>(const spirv_cross::Compiler& compiler,
                                                                     const spirv_cross::ShaderResources& spirv_resources,
                                                                     dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.sampled_images )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_IMAGE_SAMPLER;

        ret_resource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        ret_resource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }
    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_IMAGE_STORAGE>(const spirv_cross::Compiler& compiler,
                                                                     const spirv_cross::ShaderResources& spirv_resources,
                                                                     dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.storage_images )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_IMAGE_STORAGE;

        ret_resource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        ret_resource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }
    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_SAMPLER>(const spirv_cross::Compiler& compiler,
                                                               const spirv_cross::ShaderResources& spirv_resources,
                                                               dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.separate_samplers )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_SAMPLER;

        ret_resource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        ret_resource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }
    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_UNIFORM_BUFFER>(const spirv_cross::Compiler& compiler,
                                                                      const spirv_cross::ShaderResources& spirv_resources,
                                                                      dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.uniform_buffers )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_UNIFORM_BUFFER;

        ret_resource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        ret_resource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_struct_size(compiler, resource, &ret_resource);
        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }
    return true;
}

template<>
static bool read_shader_resource<gfx::SHADER_RESOURCE_STORAGE_BUFFER>(const spirv_cross::Compiler& compiler,
                                                                      const spirv_cross::ShaderResources& spirv_resources,
                                                                      dt::vector<shader_resource>* out_resources)
{
    for( auto& resource : spirv_resources.storage_buffers )
    {
        shader_resource ret_resource{ };
        ret_resource.name = resource.name;
        ret_resource.type = gfx::SHADER_RESOURCE_STORAGE_BUFFER;

        ret_resource.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        ret_resource.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        read_resource_struct_size(compiler, resource, &ret_resource);
        read_resource_array_size(compiler, resource, &ret_resource);

        out_resources->push_back(ret_resource);
    }
    return true;
}
#pragma endregion read_shader_resource<>

bool reflector::reflect_shader(gfx::shader_def* pShader, dt::vector<shader_resource>* out_resources)
{
    // CompilerGLSL is a ~10KB object. Stack should be fine but moving to a ptr to
    // keep the compiler warnings happy.
    std::vector<u32> spirv_data;
    spirv_data.resize(pShader->data.size() / sizeof(u32));
    memcpy(spirv_data.data(), pShader->data.data(), pShader->data.size());

    dt::unique_ptr<spirv_cross::CompilerGLSL> pCompiler = dt::make_unique<spirv_cross::CompilerGLSL>(spirv_data);

    spirv_data.clear();

    spirv_cross::ShaderResources spirv_resources = pCompiler->get_shader_resources();
    bool success = true;

    { success |= parse_shader_resources(*pCompiler, spirv_resources, out_resources); }
    // { success |= parse_push_constants(*pCompiler, spirv_resources, out_resources); }
    // { success |= parse_specialization_constants(*pCompiler, spirv_resources, out_resources); }

    return success;
}

bool reflector::parse_shader_resources(spirv_cross::CompilerGLSL& compiler,
                                       spirv_cross::ShaderResources& resources,
                                       dt::vector<shader_resource>* out_resources)
{
    bool success = true;
    success |= read_shader_resource<gfx::SHADER_RESOURCE_INPUT>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_INPUT_ATTACHMENT>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_OUTPUT>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_IMAGE>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_IMAGE_SAMPLER>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_IMAGE_STORAGE>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_SAMPLER>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_UNIFORM_BUFFER>(compiler, resources, out_resources);
    success |= read_shader_resource<gfx::SHADER_RESOURCE_STORAGE_BUFFER>(compiler, resources, out_resources);
    return success;
}

bool reflector::reflect_pass(gfx::program_def* pProgram, u64 passIdx, dt::vector<dt::vector<shader_resource>>& shader_resources)
{
    bool success = true;
    gfx::pass_def* pass = &pProgram->passes[passIdx];
    
    // Lets start by modifying our PSO. Vertex Input Assembly is all we need to modify.
    if( pass->stage_mask & gfx::SHADER_STAGE_VERTEX )
    {
        // I guess we're going to assume that we're supplying vertex data fully packed?
        // Pipeline requires you to define what number of buffers the vertices are split into
        // at creation time (packed position|normal buffer vs position buffer + normal buffer) which
        // is incredibly inconvenient. Realistically, I can control the mesh format so I'm just going to say
        // it has to be packed :)

        dt::vector<shader_resource>& vertexResources = shader_resources[pass->vertex_index];

        dt::vector<shader_resource> inputResources;
        u32 buffer_stride = 0;
        for( shader_resource& resource : vertexResources )
        {
            if( resource.type == gfx::SHADER_RESOURCE_INPUT )
            {
                inputResources.push_back(resource);
                buffer_stride += resource.vecSize * resource.arraySize * sizeof(f32); // assume its a 32bit float?
            }
        }

        std::qsort(
            inputResources.data(),
            inputResources.size(),
            sizeof(shader_resource),
            [](const void* a, const void* b) -> i32
            {
                const shader_resource* left = static_cast<const shader_resource*>(a);
                const shader_resource* right = static_cast<const shader_resource*>(b);

                return i32_cast(right->location) - i32_cast(left->location);
            });

        gfx::vertex_input_state new_state{ };

        // If we have any input resources, add them all into a single input description
        if( inputResources.size() > 0 )
        {
            gfx::vertex_input_description vertex_buffer_desc{ };
            vertex_buffer_desc.stride = buffer_stride;
            vertex_buffer_desc.attribute_count = u32_cast(inputResources.size());

            u32 offset = 0;
            for( u32 idx = 0; idx < u32_cast(inputResources.size()); idx++ )
            {
                shader_resource& resource = inputResources[idx];
                vertex_buffer_desc.attributes[idx].offset = offset;

                u32 resource_stride = resource.vecSize * resource.arraySize * sizeof(f32);
                offset += resource_stride;

                if( resource_stride == 2 * sizeof(f32) )
                {
                    vertex_buffer_desc.attributes[idx].format = gfx::format::R32G32_SFLOAT;
                }
                else if( resource_stride == 3 * sizeof(f32) )
                {
                    vertex_buffer_desc.attributes[idx].format = gfx::format::R32G32B32_SFLOAT;
                }
                else if( resource_stride == 4 * sizeof(f32) )
                {
                    vertex_buffer_desc.attributes[idx].format = gfx::format::R32G32B32A32_SFLOAT;
                }
            }

            new_state.channel_count = 1;
            new_state.descriptions[0] = vertex_buffer_desc;

            pass->pipeline_state_object.set_vertex_input_state(new_state);
        }
    }
    
    success |= reflect_tables(pass, shader_resources);

    return success;
}

bool reflector::reflect_tables(gfx::pass_def* pPass, dt::vector<dt::vector<shader_resource>>& all_resources)
{
    bool success = true;

    // Organise our tables
    for( u32 table_idx = 0; table_idx < gfx::DESCRIPTOR_TABLE_COUNT; table_idx++ )
    {
        // Get all resources that apply to this table
        std::vector<shader_resource> resources;
        for( u32 shader_idx = 0; shader_idx < all_resources.size(); shader_idx++ )
        {
            // Find what stage this vector means.
            gfx::shader_stage_flag_bits stage;
            if( pPass->vertex_index == shader_idx )
                stage = gfx::SHADER_STAGE_VERTEX;
            else if( pPass->geometry_index == shader_idx )
                stage = gfx::SHADER_STAGE_GEOMETRY;
            else if( pPass->fragment_index == shader_idx )
                stage = gfx::SHADER_STAGE_FRAGMENT;
            else if( pPass->compute_index == shader_idx )
                stage = gfx::SHADER_STAGE_COMPUTE;
            else
            {
                SHADEV_ASSERT(false, "Shader stage unsupported.");
            }

            for( shader_resource& resource : all_resources[shader_idx] )
            {
                // This doesn't go in a table.
                if( !gfx::is_buffer_resource_type(resource.type) && !gfx::is_image_resource_type(resource.type) )
                    continue;

                // Not the right table idx.
                if( resource.set != table_idx )
                    continue;

                resource.stages |= stage;
                resources.push_back(resource);
            }
        }

        if( resources.empty() )
            continue;

        gfx::descriptor_table_def& table = pPass->tables.emplace_back();
        success |= reflect_table(&table, resources);
    }

    return success;
}

bool reflector::reflect_table(gfx::descriptor_table_def* pTable, const std::vector<shader_resource>& all_resources)
{
    // combine the same bindings
    // sort by binding
    // insert empty slots if necessary
    // split by image/buffer
    // write to our table

    std::vector<bool> filled_slots;
    std::vector<shader_resource> resources;

    for( const shader_resource& rsc : all_resources )
    {
        if( resources.size() <= rsc.binding )
        {
            resources.resize(rsc.binding + 1);
            filled_slots.resize(rsc.binding + 1);
        }

        if( filled_slots[rsc.binding] == true )
        {
            // TODO validate we're actually the same resource..
            resources[rsc.binding].stages |= rsc.stages;
        }
        else
        {
            filled_slots[rsc.binding] = true;
            resources[rsc.binding] = rsc;
        }
    }

    for( u32 idx = 0; idx < resources.size(); idx++ )
    {
        if( filled_slots[idx] == false )
        {
            resources[idx].type = gfx::SHADER_RESOURCE_EMPTY;
        }
    }
    
    // We are sorted, we are combined, we have empty slots.. Put the resource in the table man
    for( shader_resource& resource : resources )
    {
        gfx::descriptor_slot_def slot{ };
        slot.name = resource.name;
        slot.array_count = resource.arraySize;
        slot.type = resource.type;
        slot.visibility_mask = resource.stages;

        if( gfx::is_buffer_resource_type(resource.type) )
            pTable->buffer_slots.push_back(slot);
        else
            pTable->image_slots.push_back(slot);
    }

    return true;
}

bool reflector::reflect(gfx::program_def* pProgram)
{
    bool success = true;
    dt::vector<dt::vector<shader_resource>> perShaderResources;
    perShaderResources.resize(pProgram->shaders.size());

    for( u64 idx = 0; idx < pProgram->shaders.size(); idx++ )
    {
        success |= reflect_shader(&pProgram->shaders[idx], &perShaderResources[idx]);
    }

    if( !success )
    {
        SHADEV_ERROR("Failed to reflect shader resources for program {}. See previous errors.", pProgram->name);
        return false;
    }

    for( u64 idx = 0; idx < pProgram->passes.size(); idx++ )
    {
        success |= reflect_pass(pProgram, idx, perShaderResources);
    }
    return success;
}