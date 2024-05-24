#include "DrivenStructDef.h"

REGISTER_DATA_DRIVER(STRUCTDEF_TAG, DrivenStructDef);

DrivenStructDef::DrivenStructDef(pugi::xml_node node) :
    DrivenObject(STRUCTDEF_TAG, node),
    m_name(nullptr),
    m_memberData()
{
    for( pugi::xml_attribute attr : node.attributes() )
    {
        if( !strcmp(attr.name(), "type") )
        {
            m_name = attr.value();
        }
    }

    for( pugi::xml_node child : node.children() )
    {
        m_memberData.push_back(::Reflector::get_element_object(child));
    }
}

void DrivenStructDef::serialize_parser(std::ostream& stream, size_t tab_depth)
{

}

void DrivenStructDef::serialize_header(std::ostream& stream, size_t tab_depth)
{
    append_tabs(stream, tab_depth); stream << "struct " << m_name << "\r\n";
    append_tabs(stream, tab_depth); stream << "{\r\n";
    for( auto& member : m_memberData )
    {
        member->serialize_header(stream, tab_depth + 1);
    }
    append_tabs(stream, tab_depth); stream << "};\r\n";
}