#include "path.h"

#include "helpers/string_manip.h"
#include <sstream>

namespace sys
{

path::path(const std::string& data, bool normalise) :
    m_data(data)
{
    if( normalise )
        this->normalise();
}

path::path(std::string&& data, bool normalise) :
    m_data(std::move(data))
{
    if( normalise )
        this->normalise();
}

path::path(const char* value, bool normalise) :
    m_data(value)
{
    if( normalise )
        this->normalise();
}

void path::append(path path)
{ }

void path::append(const std::string& value)
{ }

void path::append(const char* value)
{ }

void path::normalise()
{
    // character replacement pass.
    std::transform(m_data.begin(), m_data.end(), m_data.begin(), [](unsigned char c)
        {
        #if SYS_PATH_CASE_INSENSITIVE
            unsigned char out = std::tolower(c);
        #else
            unsigned char out = c;
        #endif
            if( out == SYS_PATH_ALT_SEPARATOR )
                out = SYS_PATH_SEPARATOR;
            return out;
        });

    // remove duplicate separators
    std::string trim;
    trim.reserve(m_data.size());

    bool flag = false;
    for( u64 idx = 0; idx < m_data.size(); idx++ )
    {
        if( m_data[idx] == SYS_PATH_SEPARATOR )
        {
            if( !flag )
                trim += m_data[idx];

            flag = true;
        }
        else
        {
            trim += m_data[idx];
            flag = false;
        }
    }

    m_data = trim;
}

path path::as_lower() const
{
    std::string copy = m_data;
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c)
        {
            return std::tolower(c);
        });
    return path(std::move(copy));
}

path path::without_mount() const
{
    return path("");
}

bool path::is_rooted_by(const path& other) const
{
    return false;
}

bool path::is_relative() const
{
    return false;
}

bool path::contains_segment(const char* value) const
{
    return false;
}

bool path::contains_segment(const std::string& value) const
{
    return false;
}

std::string_view path::get_extension() const
{
    std::string_view filename = get_filename();
    u64 extTokenIdx = filename.find_last_of(SYS_PATH_EXTENSION_TOKEN);

    if( extTokenIdx == filename.npos )
        return "";

    return std::string_view(filename.begin() + extTokenIdx + 1, filename.end());
}

std::string_view path::get_mount() const
{
    u64 mountTokenIdx = m_data.find_first_of(SYS_PATH_MOUNT_TOKEN);
    if( mountTokenIdx == m_data.npos )
        return "";

    return std::string_view(m_data.begin(), m_data.begin() + mountTokenIdx);
}

std::string_view path::get_filename() const
{
    return get_segment(get_segment_count() - 1);
}

std::string_view path::get_filename_without_extension() const
{
    std::string_view filename = get_filename();
    u64 extTokenIdx = filename.find_last_of(SYS_PATH_EXTENSION_TOKEN);

    if( extTokenIdx == filename.npos )
        return filename;

    return std::string_view(filename.begin(), filename.begin() + extTokenIdx - 1);
}

std::string_view path::get_directory() const
{
    std::string_view filename = get_filename();
    return std::string_view(&m_data.front(), &filename.front());
}

path path::get_directory_as_path() const
{
    std::string copy(get_directory());
    return path(copy);
}

u64 path::get_segment_count() const
{
    return occurances(m_data, SYS_PATH_SEPARATOR) + 1;
}

std::string_view path::get_segment(u64 idx) const
{
    u64 seg = 0;
    u64 start = 0;
    u64 end = m_data.size();
    
    while( seg != idx )
    {
        start = m_data.find_first_of(SYS_PATH_SEPARATOR, start) + 1;
        if( start == m_data.npos )
            QUITFMT("Invalid segment {}", idx);

        end = m_data.find_first_of(SYS_PATH_SEPARATOR, start);
        // we're in the final segment
        if( end == m_data.npos )
            end = m_data.size();

        seg++;
    }

    return std::string_view(m_data.begin() + start, m_data.begin() + end);
}

path::operator std::string() const
{
    return m_data;
}

path::operator const char*() const
{
    return m_data.c_str();
}

} // sys