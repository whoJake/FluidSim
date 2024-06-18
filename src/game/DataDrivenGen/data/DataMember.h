#pragma once

#include "DrivenObject.h"
#include "InitializerList.h"

enum class MemberVisibility
{
    PUBLIC,
    PROTECTED,
    PRIVATE
};

class DataMember : public DrivenObject
{
protected:
    DataMember(pugi::xml_node owner) :
        DrivenObject(owner)
    {
        m_name = read_required_attribute(owner, "name");

        const char* visibility = read_attribute(owner, "visibility");
        if( !strcmp(visibility, "") )
        {
            auto visIt = s_visibilityMap.find(mtl::hash_string(visibility));
            if( visIt == s_visibilityMap.end() )
            {
                throw parse_exception(owner, std::format("visibility attribute {} does not match any accepted value.", visibility));
            }

            m_visibility = visIt->second;
        }

    }

public:
    virtual std::string_view get_type_name() const = 0;
    virtual InitializerList get_initializer_list() const = 0;

    inline std::string_view get_name() const
    {
        return m_name;
    }

    inline MemberVisibility get_visibility() const
    {
        return m_visibility;
    }
private:
    std::string_view m_name;
    MemberVisibility m_visibility{ MemberVisibility::PUBLIC };

    inline static std::unordered_map<mtl::hash_string, MemberVisibility> s_visibilityMap =
    {
        { mtl::hash_string("public"),       MemberVisibility::PUBLIC },
        { mtl::hash_string("protected"),    MemberVisibility::PROTECTED },
        { mtl::hash_string("private"),      MemberVisibility::PRIVATE },
    };
};