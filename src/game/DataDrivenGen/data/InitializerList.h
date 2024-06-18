#pragma once

class InitializerNode
{
public:
    InitializerNode(std::string value) :
        m_value(value)
    { }

    virtual ~InitializerNode()
    { }

    inline virtual std::string serialize() const
    {
        return std::string(m_value);
    }

private:
    std::string m_value;
};

class InitializerList : public InitializerNode
{
public:
    InitializerList() :
        InitializerNode(""),
        m_nodes()
    { }

    InitializerList(InitializerNode node) :
        InitializerNode(""),
        m_nodes()
    {
        append_node(std::move(node));
    }

    virtual ~InitializerList()
    {
        for( InitializerNode* node : m_nodes )
        {
            delete node;
        }
    }

    inline std::string serialize() const override
    {
        if( m_nodes.size() <= 0 )
        {
            return "{ }";
        }

        std::stringstream stream;
        stream << "{ ";
        for( size_t i = 0; i < m_nodes.size() - 1; i++ )
        {
            InitializerNode* node = m_nodes[i];
            stream << node->serialize() << ", ";
        }
        stream << m_nodes.back()->serialize();
        stream << " }";

        return stream.str();
    }

    inline void append_node(InitializerNode&& node)
    {
        m_nodes.push_back(new InitializerNode(node));
    }

    inline void append_list(InitializerList&& list)
    {
        m_nodes.push_back(new InitializerList(list));
    }

private:
    std::vector<InitializerNode*> m_nodes;
};