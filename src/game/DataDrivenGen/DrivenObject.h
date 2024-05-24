#pragma once

#ifndef PUGIXML_NO_XPATH
    #define PUGIXML_NO_XPATH
#endif
#ifndef PUGIXML_HEADER_ONLY
    #define PUGIXML_HEADER_ONLY
#endif

#include "pugixml.hpp"

#include <unordered_map>
#include <memory>
#include <exception>
#include <string>
#include <format>

#define REGISTER_DATA_DRIVER(tag, name) static bool name_RegisterResult = ::Reflector::register_object(tag, [](pugi::xml_node node) -> DrivenObject* { return static_cast<DrivenObject*>(new name(node)); })
#define TAB_STR "  "

static size_t hash_tag(std::string_view tag)
{
    // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes ?
    size_t seed = 0x517cc1b727220a95;
    for( char c : tag )
    {
        std::hash<char> hasher;
        size_t hash = hasher(c);

        hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash;
    }
    return seed;
}

static void append_tabs(std::ostream& stream, size_t tab_count)
{
    for( size_t i = 0; i < tab_count; i++ )
    {
        stream << TAB_STR;
    }
}

class DrivenObject
{
public:
    virtual void serialize_parser(std::ostream& stream, size_t tab_depth) = 0;
    virtual void serialize_header(std::ostream& stream, size_t tab_depth) = 0;

    bool is_type(std::string_view other) const;
protected:
    DrivenObject() = delete;
    DrivenObject(std::string_view tag, pugi::xml_node element);

    pugi::xml_node m_element;
    size_t m_tagHash;
};

class Reflector
{
public:
    static bool register_object(const char* tag, std::function<DrivenObject*(pugi::xml_node)> constructor);
    static std::unique_ptr<DrivenObject> get_element_object(pugi::xml_node element);
private:
    inline static std::unordered_map<size_t, std::function<DrivenObject*(pugi::xml_node)>> sm_registeredObjects;
};
