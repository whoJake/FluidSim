#include "scaffold.h"

namespace fw
{

static void empty_node_func()
{ }

scaffold_startup_node scaffold::sm_startupRoot = scaffold_startup_node{ empty_node_func };
scaffold_update_node scaffold::sm_updateRoot = scaffold_update_node{ empty_node_func };
scaffold_shutdown_node scaffold::sm_shutdownRoot = scaffold_shutdown_node{ empty_node_func };

scaffold::state scaffold::sm_state = SCAFFOLD_STATE_INACTIVE;

void scaffold::startup()
{
    switch( sm_state )
    {
    case SCAFFOLD_STATE_INACTIVE:
        sm_state = SCAFFOLD_STATE_STARTUP;
        state_machine();
        break;
    default:
        SYSASSERT(false, "Cannot begin scaffold when its not inactive..");
        break;
    }
}

void scaffold::set_should_stop()
{
    switch( sm_state )
    {
    case SCAFFOLD_STATE_STARTUP:
    case SCAFFOLD_STATE_UPDATE:
    case SCAFFOLD_STATE_SHUTDOWN:
        sm_state = SCAFFOLD_STATE_SHUTDOWN;
        break;
    default:
        SYSASSERT(false, "Trying to set as should stop but we haven't started.");
        break;
    }
}

scaffold_startup_node& scaffold::add_startup_node(scaffold_startup_node node)
{
    return sm_startupRoot.add_child(node);
}

scaffold_update_node& scaffold::add_update_node(scaffold_update_node node)
{
    return sm_updateRoot.add_child(node);
}

scaffold_shutdown_node& scaffold::add_shutdown_node(scaffold_shutdown_node node)
{
    return sm_shutdownRoot.add_child(node);
}

void scaffold::state_machine()
{
    bool do_work = true;
    while( do_work )
    {
        switch( sm_state )
        {
        case SCAFFOLD_STATE_STARTUP:
            sm_state = SCAFFOLD_STATE_UPDATE;
            state_startup();
            break;
        case SCAFFOLD_STATE_UPDATE:
            state_update();
            break;
        case SCAFFOLD_STATE_SHUTDOWN:
            do_work = false;
            state_shutdown();
            break;
        case SCAFFOLD_STATE_INACTIVE:
            SYSASSERT(false, "In update loop whilst inactive. Whats wrong..");
            do_work = false;
            break;
        }
    }

    sm_state = SCAFFOLD_STATE_INACTIVE;
}

void scaffold::state_startup()
{
    std::vector<scaffold_startup_node> nodes_to_process;
    nodes_to_process.push_back(sm_startupRoot);

    while( !nodes_to_process.empty() )
    {
        scaffold_startup_node node = nodes_to_process.back();
        nodes_to_process.pop_back();

        node.invoke();
        for( scaffold_startup_node child : node.get_children() )
        {
            nodes_to_process.push_back(child);
        }
    }
}

void scaffold::state_update()
{
    std::vector<scaffold_update_node> nodes_to_process;
    nodes_to_process.push_back(sm_updateRoot);

    while( !nodes_to_process.empty() )
    {
        scaffold_update_node node = nodes_to_process.back();
        nodes_to_process.pop_back();

        node.invoke();
        for( scaffold_update_node child : node.get_children() )
        {
            nodes_to_process.push_back(child);
        }
    }
}

void scaffold::state_shutdown()
{
    std::vector<scaffold_shutdown_node> nodes_to_process;
    nodes_to_process.push_back(sm_shutdownRoot);

    while( !nodes_to_process.empty() )
    {
        scaffold_shutdown_node node = nodes_to_process.back();
        nodes_to_process.pop_back();

        node.invoke();
        for( scaffold_shutdown_node child : node.get_children() )
        {
            nodes_to_process.push_back(child);
        }
    }
}

} // fw