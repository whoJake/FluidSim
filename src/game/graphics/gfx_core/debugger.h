#pragma once

#include <vector>
#include <functional>

namespace gfx
{

class debugger
{
public:
    enum class severity
    {
        none,
        verbose,
        info,
        warning,
        error,
    };

    using callback = std::function<void(severity, const char*)>;

    debugger() = default;
    ~debugger() = default;
    
    DEFAULT_COPY(debugger);
    DEFAULT_MOVE(debugger);

    void attach_callback(callback func);

    void send_event(severity level, const char* message);

    void set_impl_ptr(void* data);
    void* get_impl_ptr() const;
private:
    std::vector<callback> m_callbacks;
    void* m_impl{ nullptr };
};

} // gfx