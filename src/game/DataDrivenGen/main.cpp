#include "data/DrivenObject.h"
#include "data/types/DrivenFloat.h"
#include <iostream>

int main(int argc, const char* argv[])
{
    pugi::xml_document doc;
    doc.load_file("tests/utStructWithMember.xml");

    for( pugi::xml_node node : doc.children() )
    {
        DrivenFloat data(node);
        std::cout << data.get_initializer_list().serialize() << std::endl;
    }

    return -1;
}