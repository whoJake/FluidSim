#include "Params.h"
#include <vector>

#include <sstream>

const char* g_executable{ nullptr };
static std::vector<const char*> s_args{ };

Param::Param(const char* name) :
    m_name(name)
{ }

bool Param::get() const
{
    for( const char* arg : s_args )
    {
        if( !strncmp(arg, m_name, strlen(m_name)) )
        {
            return true;
        }
    }
    return false;
}

const char* Param::value() const
{
    for( const char* arg : s_args )
    {
        if( !strncmp(arg, m_name, strlen(m_name)) )
        {
            const char* equal = strchr(arg, '=');
            if( equal )
            {
                return equal + 1;
            }
            return equal;
        }
    }
    return nullptr;
}

bool Param::get_float(float* out) const
{ 
    double temp{ };
    bool result = get_double(&temp);
    if( !result )
    {
        return false;
    }

    *out = static_cast<float>(temp);
    return true;
}

bool Param::get_int(int* out) const
{
    const char* str = value();
    if( str == nullptr )
    {
        return false;
    }

    *out = atoi(str);
    return true;
}

bool Param::get_double(double* out) const
{
    const char* str = value();
    if( str == nullptr )
    {
        return false;
    }

    *out = atof(str);
    return true;
}

void Param::set_executable(const char* name)
{
    g_executable = name;
}

void Param::add_params(int argc, const char* argv[])
{
    for( int i = 0; i < argc; i++ )
    {
        if( argv[i][0] == '-' )
        {
            s_args.push_back(argv[i] + 1);
        }
    }

    s_args.shrink_to_fit();
}

std::string Param::get_command_line()
{
    std::stringstream stream;
    stream << g_executable;
    for( const char* arg : s_args )
    {
        stream << " -" << arg;
    }
    
    return stream.str();
}