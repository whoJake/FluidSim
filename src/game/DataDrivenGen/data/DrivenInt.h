#pragma once

#include "DrivenObject.h"

#define INT_TAG "int"

class DrivenInt : public DrivenObject
{
public:
    DrivenInt() = delete;
    DrivenInt(pugi::xml_node element);

    void serialize_parser(std::ostream& stream, size_t tab_depth) override;
    void serialize_header(std::ostream& stream, size_t tab_depth) override;
private:
};