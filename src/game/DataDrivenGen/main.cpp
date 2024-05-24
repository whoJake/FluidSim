#include "DrivenObject.h"
#include <iostream>

int main(int argc, const char* argv[])
{
    pugi::xml_document doc;
    doc.load_file("tests/utStructWithMemberInNamespace.xml");

    for( pugi::xml_node node : doc.children() )
    {
        std::unique_ptr<DrivenObject> obj = ::Reflector::get_element_object(node);
        obj->serialize_header(std::cout, 0);
    }

    return -1;
}