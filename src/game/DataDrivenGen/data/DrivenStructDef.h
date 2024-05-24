#pragma once

#include "DrivenObject.h"

#define STRUCTDEF_TAG "structdef"

class DrivenStructDef : public DrivenObject
{
public:
    DrivenStructDef() = delete;
    DrivenStructDef(pugi::xml_node node);

    void serialize_parser(std::ostream& stream, size_t tab_depth) override;
    void serialize_header(std::ostream& stream, size_t tab_depth) override;
private:
    const char* m_name;
    std::vector<std::unique_ptr<DrivenObject>> m_memberData;
};