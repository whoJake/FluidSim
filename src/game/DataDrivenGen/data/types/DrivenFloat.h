#pragma once

#include "data/DataMember.h"

class DrivenFloat : public DataMember
{
public:
    DrivenFloat() = delete;
    DrivenFloat(pugi::xml_node owner) :
        DataMember(owner)
    {
        m_initializedValue = read_attribute_as_float(owner, "init");
    }

    inline std::string_view get_type_name() const override
    {
        return "float";
    }

    inline InitializerList get_initializer_list() const override
    {
        std::string retval = std::format("{}", m_initializedValue);
        float tmp;
        if( std::modf(m_initializedValue, &tmp) == 0.f )
        {
            // 2f is invalid, 2.f is correct
            retval += '.';
        }
        retval += 'f';

        return { retval };
    }
private:
    float m_initializedValue{ };
};