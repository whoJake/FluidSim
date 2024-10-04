#include "hash_string.h"
#include "data/fixed_vector.h"

namespace sys
{

using entry = std::pair<hash_string_value, const char*>;
using table = std::vector<entry>;


mtl::fixed_vector<table> m_hashTables(u64_cast(hash_string_pools::pool_count));

table& get_pool_table(hash_string_pools pool)
{
    return m_hashTables[u64_cast(pool)];
}

std::string_view lookup_in_table(const table& tbl, hash_string_value val)
{
    // TODO binary search
    for( const entry& ent : tbl )
    {
        if( ent.first == val )
        {
            return std::string_view(ent.second);
        }
    }

    return std::string_view("");
}

void hash_string_table::insert(hash_string_pools pool, hash_string_value hash, std::string_view string)
{
    table& tbl = get_pool_table(pool);
    
    // TODO don't add if already exists
    // TODO sorted entry
    char* copy = new char[string.size() + 1];
    memcpy(copy, string.data(), string.size());
    copy[string.size()] = '\0';

    tbl.push_back(std::make_pair(hash, copy));
}

std::string_view hash_string_table::lookup(hash_string_pools pool, hash_string_value hash)
{
    table& tbl = get_pool_table(pool);
    return lookup_in_table(tbl, hash);
}

} // sys