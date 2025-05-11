#pragma once
#include <functional>

namespace fw
{

class update_graph;
using update_graph_node_function = std::function<void()>;

class update_graph_node
{
public:
    friend class update_graph;

private:
    update_graph_node_function m_cb;
};

} // fw