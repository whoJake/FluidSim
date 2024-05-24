#pragma once

#include "DrivenObject.h"

#define FLOAT_TAG "float"

class DrivenFloat : public DrivenObject
{
public:
    DrivenFloat() = delete;
    DrivenFloat(pugi::xml_node element);

    void serialize_parser(std::ostream& stream, size_t tab_depth) override;
    void serialize_header(std::ostream& stream, size_t tab_depth) override;
private:
    const char* m_name;
    float m_init{ };
};