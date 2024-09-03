#include "app.h"

#include "system/param.h"
#include "system/details/basic_log.h"
#include "system/details/log_console.h"
#include "system/details/log_mt.h"

MAKEPARAM(no_log);

namespace fw
{

i32 app::run(i32 argc, const char* argv[])
{
    std::vector<const char*> args;

    // first arg is always* gonna be path of executable so lets just ignore it.
    for( i32 idx = 1; idx < argc; idx++ )
    {
        args.push_back(argv[idx]);
    }
    sys::param::init(args);

    bool success = true;
    if( !p_no_log.get() )
    {
        success |= sys::log::initialise(make_log()) == 1;
    }
    else
    {
        success |= sys::log::initialise(new sys::log::details::basic_log(sys::log::level::disable)) == 1;
    }

    if( !success )
    {
        EXITCODE(app_exitcodes::init_failure);
    }

    success |= on_startup();
    if( !success )
    {
        EXITCODE(app_exitcodes::init_failure);
    }

    i32 exitcode = app_main();

    on_shutdown();

    sys::log::shutdown();
    return exitcode;
}

sys::log::details::log_manager* app::make_log() const
{
    sys::log::details::logger logger;
    logger.register_target(new sys::log::details::console_target());

    return new sys::log::details::log_mt(std::move(logger), sys::log::level::none, 8192, 1);
}

bool app::on_startup()
{
    return true;
}

void app::on_shutdown()
{ }

} // fw