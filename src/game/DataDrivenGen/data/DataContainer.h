#pragma once

#include "DrivenObject.h"

class DataContainer : public DrivenObject
{
protected:
    DataContainer(pugi::xml_node owner) :
        DrivenObject(owner)
    { 
        m_type = read_required_attribute(owner, "type");
        m_constructable = read_attribute_as_bool(owner, "constructable", true);
    }

public:
    inline std::string_view get_type() const
    {
        return m_type;
    }
private:
    std::string_view m_type;
    bool m_constructable{ true };
};