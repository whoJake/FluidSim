#pragma once

inline std::vector<std::string> split_string(const std::string& string, std::string delimitter)
{
    std::vector<std::string> retval;

    std::stringstream ss;
    for( size_t i = 0; i < string.length(); i++ )
    {
        const char* cstr = string.data() + i;
        size_t len = strlen(cstr);

        if( len >= delimitter.length() )
        {
            if( !strncmp(cstr, delimitter.c_str(), delimitter.length()) )
            {
                retval.push_back(ss.str());
                ss.str(std::string());
                i += delimitter.length();
            }
        }
        if( i < string.length() )
        {
            cstr = string.data() + i;
            ss << *cstr;
        }
    }

    if( ss.str().length() != 0 )
        retval.push_back(ss.str());
    return retval;
}