#include "DrivenObject.h"

bool Reflector::register_object(const char* tag, std::function<DrivenObject*(pugi::xml_node)> constructor)
{
    // doesn't include null terminator
    size_t tagHash = hash_tag(std::string_view(tag));
    sm_registeredObjects.emplace(tagHash, constructor);
    return true;
}

std::unique_ptr<DrivenObject> Reflector::get_element_object(pugi::xml_node element)
{
    const char* nodeName = element.name();
    size_t nameHash = hash_tag(std::string_view(nodeName));

    auto it = sm_registeredObjects.find(nameHash);
    if( it == sm_registeredObjects.end() )
    {
        throw std::exception(std::format("Object name {} is not recognised.", nodeName).c_str());
    }

    auto& constructor = sm_registeredObjects[nameHash];
    return std::unique_ptr<DrivenObject>(constructor(element));
}

DrivenObject::DrivenObject(std::string_view tag, pugi::xml_node element) :
    m_element(element),
    m_tagHash(hash_tag(tag))
{ }

bool DrivenObject::is_type(std::string_view other) const
{
    return m_tagHash == hash_tag(other);
}