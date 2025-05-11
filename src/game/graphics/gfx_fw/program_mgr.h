#pragma once
#include "dt/vector.h"
#include "dt/unique_ptr.h"
#include "dt/hash_string.h"
#include <string>

namespace gfx
{

class descriptor_table_desc;
class program;

struct program_def;

/// Descriptor Cache : Handles descriptor_table_descs.
/// In order for a descriptor_table_desc to gain its impl ptr it must pass through the descriptor
/// cache, which will use a pre-cached descriptor_table_desc if it has already been created.
/// 
/// TODO: API could be nicer on this. I like passing in a pre-formed (but without an impl) descriptor_table_desc
/// into the cache in order to gain an impl ptr but it feels like a weird way of doing it.
class descriptor_cache
{
public:
    descriptor_cache() = default;
    ~descriptor_cache() = default;

    DELETE_COPY(descriptor_cache);
    DELETE_MOVE(descriptor_cache);

    void destroy();

    descriptor_table_desc* get_descriptor_table_desc(descriptor_table_desc&& desc);
private:
    dt::vector<u64> m_tableDescHashes;
    dt::vector<descriptor_table_desc> m_tableDescs;
};

class program_mgr
{
public:
    static void initialise(const char* base_directory);
    static void shutdown();

    static const program* find_program(dt::hash_string32 name);
    static void load(const char* path);
private:
    static program* insert(dt::unique_ptr<program>&& prog);
    static program convert_from(const program_def& program_def);

    inline static program_mgr* sm_instance{ nullptr };

private:
    std::string m_baseDirectory;
    dt::vector<dt::unique_ptr<program>> m_loadedPrograms;
    descriptor_cache m_cache;
};

} // gfx