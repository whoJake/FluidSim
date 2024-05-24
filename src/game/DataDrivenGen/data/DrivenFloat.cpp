#include "DrivenFloat.h"

#include <iostream>

REGISTER_DATA_DRIVER(FLOAT_TAG, DrivenFloat);

DrivenFloat::DrivenFloat(pugi::xml_node node) :
    DrivenObject(FLOAT_TAG, node),
    m_name(nullptr)
{
    for( pugi::xml_attribute attr : node.attributes() )
    {
        if( !strcmp(attr.name(), "name") )
        {
            m_name = attr.value();
        }
        else if( !strcmp(attr.name(), "init") )
        {
            m_init = attr.as_float();
        }
    }
}

void DrivenFloat::serialize_parser(std::ostream& stream, size_t tab_depth)
{

}

void DrivenFloat::serialize_header(std::ostream& stream, size_t tab_depth)
{
    append_tabs(stream, tab_depth); stream << "float " << m_name;
    stream << "{ " << m_init << " };\r\n";
}