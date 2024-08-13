#pragma once

#include <random>

namespace fw
{

struct EntityId
{
    static constexpr u64 invalid_id = 0;
    u64 value = invalid_id;

    inline bool is_valid() const
    {
        return value == invalid_id;
    }

    static EntityId generate(std::mt19937_64& source)
    {
        inline static std::uniform_int_distribution<std::mt19937_64::result_type> distribution(1u, u64_max);
        return { distribution(source) };
    }
};

} // fw