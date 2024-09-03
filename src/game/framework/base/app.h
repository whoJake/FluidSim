#pragma once

#include "system/log.h"

namespace fw
{

enum class app_exitcodes
{
    success = 0,
    init_failure = 1,
};

#define EXITCODE(val) i32_cast(val)

class app
{
public:
    app() = default;
    virtual ~app() = default;

    DELETE_COPY(app);
    DELETE_MOVE(app);

    i32 run(i32 argc, const char* argv[]);

    virtual i32 app_main() = 0;

    virtual bool on_startup();
    virtual void on_shutdown();

    virtual sys::log::details::log_manager* make_log() const;
};

} // fw