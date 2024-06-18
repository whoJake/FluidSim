#include "Definitions.h"

#include "types/DrivenFloat.h"

const std::unordered_map<mtl::hash_string, DrivenTypes> s_definedTypeMap =
{
    { mtl::hash_string("namespace"),    DrivenTypes::NAMESPACE },
    { mtl::hash_string("classdef"),     DrivenTypes::CLASSDEF },
    { mtl::hash_string("class"),        DrivenTypes::CLASS },
    { mtl::hash_string("structdef"),    DrivenTypes::STRUCTDEF },
    { mtl::hash_string("struct"),       DrivenTypes::STRUCT },

    { mtl::hash_string("float"),        DrivenTypes::FLOAT },
    { mtl::hash_string("int"),          DrivenTypes::INT32 },
};

#define CONSTRUCTOR_FUNC(name) [](pugi::xml_node node) { return new name(node); }

const std::unordered_map<DrivenTypes, driven_type_constructor> s_definedTypeConstructors =
{
    // { DrivenTypes::NAMESPACE,   CONSTRUCTOR_FUNC(DrivenNamespace) },
    // { DrivenTypes::STRUCTDEF,   CONSTRUCTOR_FUNC(DrivenStructDef) },

    { DrivenTypes::FLOAT,       CONSTRUCTOR_FUNC(DrivenFloat) },
    // { DrivenTypes::INT32,       CONSTRUCTOR_FUNC(DrivenInt) },
};