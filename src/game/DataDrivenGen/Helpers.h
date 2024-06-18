#pragma once

#include "pugi_include.h"
#include <exception>
#include <functional>
#include <string>
#include <format>

#define TAB_STR "  "
#define ENDL_CHAR "\r\n"

class parse_exception : public std::exception
{
public:
    parse_exception(pugi::xml_node node, const std::string& message) :
        std::exception(),
        m_data(std::format("character {} : {}", node.offset_debug(), message))
    { }

    const char* what() const override
    {
        return m_data.c_str();
    }

private:
    std::string m_data;
};

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

static const char* read_attribute(pugi::xml_node node, const char* attribute_name, const char* default_value = "")
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.value();
        }
    }
    return default_value;
}

[[nodiscard]]
static const char* read_required_attribute(pugi::xml_node node, const char* attribute_name)
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.value();
        }
    }

    throw parse_exception(node, std::format(
        "Required attribute {} does not exist on node {}",
        attribute_name,
        node.name()).c_str());
}

[[nodiscard]]
static float read_attribute_as_float(pugi::xml_node node, const char* attribute_name, float default_value = 0.f)
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.as_float();
        }
    }
    return default_value;
}

[[nodiscard]]
static float read_required_attribute_as_float(pugi::xml_node node, const char* attribute_name)
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.as_float();
        }
    }

    throw parse_exception(node, std::format(
        "Required float attribute {} does not exist on node {}",
        attribute_name,
        node.name()).c_str());
}

[[nodiscard]]
static int read_attribute_as_int(pugi::xml_node node, const char* attribute_name, int default_value = 0)
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.as_int();
        }
    }
    return default_value;
}

[[nodiscard]]
static int read_required_attribute_as_int(pugi::xml_node node, const char* attribute_name)
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.as_int();
        }
    }

    throw parse_exception(node, std::format(
        "Required int attribute {} does not exist on node {}",
        attribute_name,
        node.name()).c_str());
}

[[nodiscard]]
static bool read_attribute_as_bool(pugi::xml_node node, const char* attribute_name, bool default_value = false)
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.as_bool();
        }
    }
    return default_value;
}

[[nodiscard]]
static bool read_required_attribute_as_bool(pugi::xml_node node, const char* attribute_name)
{
    for( pugi::xml_attribute attribute : node.attributes() )
    {
        if( !strcmp(attribute.name(), attribute_name) )
        {
            return attribute.as_bool();
        }
    }

    throw parse_exception(node, std::format(
        "Required bool attribute {} does not exist on node {}",
        attribute_name,
        node.name()).c_str());
}

static void parser_begin_scope(std::ostream& stream, size_t& tab_depth)
{
    append_tabs(stream, tab_depth); stream << "{" << ENDL_CHAR;
    tab_depth++;
}

static void parser_end_scope(std::ostream& stream, size_t& tab_depth)
{
    tab_depth--;
    append_tabs(stream, tab_depth); stream << "}" << ENDL_CHAR;
}

static void parser_for_loop(std::ostream& stream, size_t& tab_depth, const char* for_loop_def, std::function<void(void)> contents_func)
{
    append_tabs(stream, tab_depth); stream << "for( " << for_loop_def << " )" << ENDL_CHAR;
    parser_begin_scope(stream, tab_depth);
    contents_func();
    parser_end_scope(stream, tab_depth);
}

static void parser_for_loop(std::ostream& stream, size_t& tab_depth, std::function<void(void)> for_loop_def, std::function<void(void)> contents_func)
{
    append_tabs(stream, tab_depth); stream << "for( "; for_loop_def(); stream << " )" << ENDL_CHAR;
    parser_begin_scope(stream, tab_depth);
    contents_func();
    parser_end_scope(stream, tab_depth);
}

static void parser_if_block(std::ostream& stream, size_t& tab_depth, const char* if_def, std::function<void(void)> contents_func)
{
    append_tabs(stream, tab_depth); stream << "if( " << if_def << " )" << ENDL_CHAR;
    parser_begin_scope(stream, tab_depth);
    contents_func();
    parser_end_scope(stream, tab_depth);
}

static void parser_if_block(std::ostream& stream, size_t& tab_depth, std::function<void(void)> if_def, std::function<void(void)> contents_func)
{
    append_tabs(stream, tab_depth); stream << "if( "; if_def(); stream << " )" << ENDL_CHAR;
    parser_begin_scope(stream, tab_depth);
    contents_func();
    parser_end_scope(stream, tab_depth);
}

static void parser_search_attribute(std::ostream& stream, const char* attribute_name, size_t& tab_depth, std::function<void(void)> contents_func)
{
    parser_if_block(stream, tab_depth,
        [&stream, attribute_name]{ stream << "!strcmp(attribute.name(), \"" << attribute_name << "\")"; },
        contents_func);
}

static void parser_assign_member_to_attribute(std::ostream& stream, const char* member_name, size_t& tab_depth, const char* default_value)
{
    parser_search_attribute(stream, member_name, tab_depth, [&stream, &tab_depth, member_name, default_value]()
        {
            append_tabs(stream, tab_depth); stream << "target->" << member_name << " = attribute.value()" << ENDL_CHAR;
            parser_if_block(stream, tab_depth, 
                [&stream, member_name](){ stream << "target->" << member_name << " == \"\""; },
                [&stream, &tab_depth, member_name, default_value]()
                {
                    append_tabs(stream, tab_depth); stream << "target->" << member_name << " = \"" << default_value << "\";" << ENDL_CHAR;
                });
        });
}

static void parser_assign_member_to_attribute_as_float(std::ostream& stream, const char* member_name, size_t& tab_depth, float default_value)
{
    parser_search_attribute(stream, member_name, tab_depth, [&stream, &tab_depth, member_name, default_value]()
        {
            append_tabs(stream, tab_depth); stream << "target->" << member_name << " = attribute.as_float(static_cast<float>(" << default_value << "));" << ENDL_CHAR;
        });
}

static void parser_assign_member_to_attribute_as_int(std::ostream& stream, const char* member_name, size_t& tab_depth, int default_value)
{
    parser_search_attribute(stream, member_name, tab_depth, [&stream, &tab_depth, member_name, default_value]()
        {
            append_tabs(stream, tab_depth); stream << "target->" << member_name << " = attribute.as_int(static_cast<int>(" << default_value << "));" << ENDL_CHAR;
        });
}