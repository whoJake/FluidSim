#pragma once

#ifndef SYS_PATH_SEPARATOR
#define SYS_PATH_SEPARATOR '/'
#endif

#ifndef SYS_PATH_ALT_SEPARATOR
#define SYS_PATH_ALT_SEPARATOR '\\'
#endif

#ifndef SYS_PATH_MOUNT_TOKEN
#define SYS_PATH_MOUNT_TOKEN ':'
#endif

#ifndef SYS_PATH_CASE_INSENSITIVE
#define SYS_PATH_CASE_INSENSITIVE 1
#endif

#ifndef SYS_PATH_EXTENSION_TOKEN
#define SYS_PATH_EXTENSION_TOKEN '.'
#endif

namespace sys
{

class path
{
public:
    path(const std::string& data, bool normalise = true);
    path(std::string&& data, bool normalise = true);
    path(const char* value, bool normalise = true);

    DEFAULT_COPY(path);
    DEFAULT_MOVE(path);

    void append(path path);
    void append(const std::string& value);
    void append(const char* value);

    void normalise();
    path as_lower() const;
    path without_mount() const;

    bool is_rooted_by(const path& other) const;
    bool is_relative() const;

    bool contains_segment(const char* value) const;
    bool contains_segment(const std::string& value) const;

    std::string_view get_extension() const;
    std::string_view get_mount() const;
    std::string_view get_filename() const;
    std::string_view get_filename_without_extension() const;
    std::string_view get_directory() const;
    path get_directory_as_path() const;

    u64 get_segment_count() const;
    std::string_view get_segment(u64 idx) const;

    const char* c_str() const;

    operator std::string() const;
    operator const char*() const;
private:
    std::string m_data;
};

} // sys