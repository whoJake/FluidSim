#pragma once

#include "pugi_include.h"

// Shared includes
#include "Helpers.h"
#include <unordered_map>
#include "data/hash_string.h"

class DrivenObject
{
public:
protected:
    DrivenObject() = delete;
    DrivenObject(pugi::xml_node owner) :
        m_owner(owner)
    { }
private:
    pugi::xml_node m_owner;
};
