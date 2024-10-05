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

std::pair<bool, table::const_iterator> binary_find(const table& tbl, hash_string_value value)
{
    if( tbl.size() == 0 )
    {
        return std::make_pair(false, tbl.cend());
    }

    table::const_iterator res = std::upper_bound(
        tbl.cbegin(),
        tbl.cend(),
        value,
        [](hash_string_value val, const entry& ent)
        {
            // Not sure why this is equals, I guess we return false if its not found anyway so its fine?
            return val <= ent.first;
        });

    // Make sure not to check the value if we're at the end.
    if( res == tbl.cend() )
        return std::make_pair(false, res);

    return std::make_pair(res->first == value, res);
}

std::string_view lookup_in_table(const table& tbl, hash_string_value val)
{
    std::pair<bool, table::const_iterator> result = binary_find(tbl, val);
    if( !result.first )
    {
        // value isn't in our hash_table.
        return std::string_view("");
    }

    return std::string_view(result.second->second);
}

void hash_string_table::insert(hash_string_pools pool, hash_string_value hash, std::string_view string)
{
    table& tbl = get_pool_table(pool);
    std::pair<bool, table::const_iterator> ins = binary_find(tbl, hash);
    if( ins.first )
    {
        // value is already in the hash table, nothing for us to do.
        return;
    }

    // make a copy.
    char* copy = new char[string.size() + 1];
    memcpy(copy, string.data(), string.size());
    copy[string.size()] = '\0';

    // Insert in our sorted position.
    // This insert is incredible slow because of the amount of allocations it potentially has to do, maybe paginate this?
    tbl.insert(ins.second, std::make_pair(hash, copy));
}

std::string_view hash_string_table::lookup(hash_string_pools pool, hash_string_value hash)
{
    table& tbl = get_pool_table(pool);
    return lookup_in_table(tbl, hash);
}

} // sys