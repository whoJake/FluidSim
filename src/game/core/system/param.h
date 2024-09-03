#pragma once

namespace sys
{

#define MAKEPARAM(name) ::sys::param p_##name(#name)
#define MAKEGPARAM(name) inline static ::sys::param gp_##name = ::sys::param(#name)

class param
{
public:
    param(const char* name);
    ~param() = default;

    bool get() const;

    const char* as_value() const;
    f32 as_f32() const;
    f64 as_f64() const;

    u32 as_u32() const;
    i32 as_i32() const;

    u64 as_u64() const;
    i64 as_i64() const;
public:
    static void init(const std::vector<const char*>& args);
    static void include(const char* filename);

    static const std::vector<const char*>& get_registered_args();
private:
    static const char* find_arg(const char* search_arg, u64 length);
    static const char* extract_value(const char* arg);
private:
    const char* m_name;
    u64 m_length;
};

} // sys