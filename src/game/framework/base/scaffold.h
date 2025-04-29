#pragma once
#include <functional>

namespace fw
{

template<typename node_func>
class scaffold_node
{
public:
    scaffold_node(node_func func) :
        m_func(func),
        m_children()
    { }

    ~scaffold_node() = default;

    DEFAULT_MOVE(scaffold_node);
    DEFAULT_COPY(scaffold_node);

    scaffold_node& add_child(scaffold_node child)
    {
        return m_children.emplace_back(child);
    }

    const std::vector<scaffold_node>& get_children() const
    {
        return m_children;
    }

    template<typename... Args>
    void invoke(Args&&... args)
    {
        m_func(std::forward<Args>(args)...);
    }
private:
    node_func m_func;
    std::vector<scaffold_node> m_children;
};

using scaffold_startup_func = std::function<void()>;
using scaffold_startup_node = scaffold_node<scaffold_startup_func>;

using scaffold_update_func = std::function<void()>;
using scaffold_update_node = scaffold_node<scaffold_update_func>;

using scaffold_shutdown_func = std::function<void()>;
using scaffold_shutdown_node = scaffold_node<scaffold_shutdown_func>;

class scaffold
{
public:
    static void startup();
    static void set_should_stop();

    static scaffold_startup_node& add_startup_node(scaffold_startup_node node);
    static scaffold_update_node& add_update_node(scaffold_update_node node);
    static scaffold_shutdown_node& add_shutdown_node(scaffold_shutdown_node node);
private:
    static void state_machine();

    static void state_startup();
    static void state_update();
    static void state_shutdown();
private:
    static scaffold_startup_node sm_startupRoot;
    static scaffold_update_node sm_updateRoot;
    static scaffold_shutdown_node sm_shutdownRoot;

    enum state
    {
        SCAFFOLD_STATE_INACTIVE,

        SCAFFOLD_STATE_STARTUP,
        SCAFFOLD_STATE_UPDATE,
        SCAFFOLD_STATE_SHUTDOWN,
    };
    
    static state sm_state;
};

} // fw