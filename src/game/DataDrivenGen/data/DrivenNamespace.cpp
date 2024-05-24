#include "DrivenNamespace.h"

REGISTER_DATA_DRIVER(NAMESPACE_TAG, DrivenNamespace);

DrivenNamespace::DrivenNamespace(pugi::xml_node node) :
    DrivenObject(NAMESPACE_TAG, node),
    m_namespace(nullptr),
    m_memberData()
{
    for( pugi::xml_attribute attr : node.attributes() )
    {
        if( !strcmp(attr.name(), "value") )
        {
            m_namespace = attr.value();
        }
    }

    for( pugi::xml_node child : node.children() )
    {
        m_memberData.push_back(::Reflector::get_element_object(child));
    }
}

void DrivenNamespace::serialize_parser(std::ostream& stream, size_t tab_depth)
{

}

void DrivenNamespace::serialize_header(std::ostream& stream, size_t tab_depth)
{
    append_tabs(stream, tab_depth); stream << "namespace " << m_namespace << "\r\n";
    append_tabs(stream, tab_depth); stream << "{" << "\r\n";
    for( auto& member : m_memberData )
    {
        member->serialize_header(stream, tab_depth+1);
    }
    append_tabs(stream, tab_depth); stream << "}" << "\r\n";
}