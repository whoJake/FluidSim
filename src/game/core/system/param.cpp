#include "param.h"

namespace sys
{

namespace
{
std::vector<const char*> s_args{ };
} //

const char* param::find_arg(const char* search_arg, u64 length)
{
    for( const char* arg : s_args )
    {
        if( !strncmp(arg, search_arg, length) )
        {
            return arg;
        }
    }

    return nullptr;
}

const char* param::extract_value(const char* arg)
{
    const char* separator = strchr(arg, '=');
    if( separator )
    {
        return separator + 1;
    }

    return nullptr;
}

param::param(const char* name) :
    m_name(name),
    m_length(strlen(name))
{ }

bool param::get() const
{
    return find_arg(m_name, m_length) != nullptr;
}

const char* param::as_value() const
{
    const char* arg = find_arg(m_name, m_length);
    if( arg )
    {
        return extract_value(arg);
    }

    return nullptr;
}

f32 param::as_f32() const
{
    return f32_cast(as_f64());
}

f64 param::as_f64() const
{
    const char* value = as_value();
    if( value )
    {
        return atof(value);
    }

    return -1.0;
}

u32 param::as_u32() const
{
    const char* value = as_value();
    if( value )
    {
        return u32_cast(as_u64());
    }

    return 0;
}

i32 param::as_i32() const
{
    const char* value = as_value();
    if( value )
    {
        return atoi(value);
    }

    return 0;
}

u64 param::as_u64() const
{
    const char* value = as_value();
    if( value )
    {
        char* end;
        u64 result = strtoull(value, &end, 10);
        if( value != end )
        {
            return result;
        }
    }

    return 0;
}

i64 param::as_i64() const
{
    const char* value = as_value();
    if( value )
    {
        char* end;
        i64 result = strtoll(value, &end, 10);
        if( value != end )
        {
            return result;
        }
    }

    return 0;
}

void param::init(const std::vector<const char*>& args)
{
    std::vector<const char*> includeList;

    for( const char* arg : args )
    {
        if( *arg == '&' )
        {
            bool already_included = false;
            for( const char* include : includeList )
            {
                if( !strcmp(include, arg + 1) )
                {
                    already_included = true;
                    break;
                }
            }

            if( already_included )
            {
                continue;
            }

            include(arg + 1);
            continue;
        }

        const char* trimmed = arg;
        while( *trimmed == '-' || *trimmed == '\0' )
        {
            trimmed += 1;
        }

        if( !(*trimmed) )
        {
            // 0 length arg
            continue;
        }

        s_args.push_back(trimmed);
    }
}

void param::include(const char* filename)
{
    // TODO
}

const std::vector<const char*>& param::get_registered_args()
{
    return s_args;
}

} // sys