#pragma once

#include "logging/jclog.h"

extern jclog::Log* g_singleThreadedLog;

inline int init_global_log()
{
    if( g_singleThreadedLog )
    {
        return -1;
    }

    g_singleThreadedLog = new jclog::Log();
    g_singleThreadedLog->register_target(new jclog::ConsoleTarget());
    return 0;
}