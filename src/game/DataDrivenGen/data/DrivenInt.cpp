#include "DrivenInt.h"

#include <iostream>

REGISTER_DATA_DRIVER(INT_TAG, DrivenInt);

DrivenInt::DrivenInt(pugi::xml_node node) :
    DrivenObject(INT_TAG, node)
{
    std::cout << "Found int with value " << node.text().as_int() << std::endl;
}

void DrivenInt::serialize_parser(std::ostream& stream, size_t tab_depth)
{

}

void DrivenInt::serialize_header(std::ostream& stream, size_t tab_depth)
{

}