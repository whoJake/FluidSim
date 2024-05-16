#pragma once

#include <string>

#define PARAM(name) inline ::Param Param_##name(#name)

#ifdef CFG_DEBUG
    #define DEBUG_PARAM(name) PARAM(name)
#else
    #define DEBUG_PARAM(name)
#endif

extern const char* g_executable;

class Param
{
public:
    Param(const char* name);

    bool get() const;

    const char* value() const;
    bool get_float(float* out) const;
    bool get_int(int* out) const;
    bool get_double(double* out) const;

    static void set_executable(const char* name);
    static void add_params(int argc, const char* argv[]);
    static std::string get_command_line();
private:
    const char* m_name;
};