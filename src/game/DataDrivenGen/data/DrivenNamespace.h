#pragma once

#include "DrivenObject.h"

#define NAMESPACE_TAG "namespace"

class DrivenNamespace : public DrivenObject
{
public:
    DrivenNamespace() = delete;
    DrivenNamespace(pugi::xml_node node);

    void serialize_parser(std::ostream& stream, size_t tab_depth) override;
    void serialize_header(std::ostream& stream, size_t tab_depth) override;
private:
    const char* m_namespace;
    std::vector<std::unique_ptr<DrivenObject>> m_memberData;
};