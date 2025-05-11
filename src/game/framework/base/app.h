#pragma once

namespace fw
{

enum class app_exitcodes
{
    success = 0,
    init_failure = 1,
};

#define EXITCODE(val) i32_cast(val)
#define EXIT_SUCCESS EXITCODE(::fw::app_exitcodes::success)
#define EXIT_INIT_FAILURE EXITCODE(::fw::app_exitcodes::init_failure)

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
};

} // fw