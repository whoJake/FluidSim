#pragma once
#include <unordered_map>
#include <functional>
#include <memory>

#include "data/hash_string.h"
#include "pugi_include.h"
#include "DrivenObject.h"

enum class DrivenTypes
{
    UNKNOWN = 0,

    NAMESPACE,
    CLASSDEF,
    CLASS,
    STRUCTDEF,
    STRUCT,

    FLOAT,
    INT32,
};

using driven_type_constructor = std::function<DrivenObject*(pugi::xml_node)>;

extern const std::unordered_map<mtl::hash_string, DrivenTypes> s_definedTypeMap;
extern const std::unordered_map<DrivenTypes, driven_type_constructor> s_definedTypeConstructors;
