#include "app.h"

#include "system/param.h"
#include "system/assert.h"
#include "system/memory.h"
#include "memory_zone.h" // gfx

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

    if( p_no_log.get() )
    {
        sys::channel::get_global_channel()->set_severity(sys::SEVERITY_DISABLE);
    }

    // Should this be here or in game code/mmain? Seems fine here for now..
    sys::zone_allocator::max_zones = MEMZONE_SYSTEM_COUNT + MEMZONE_GFX_COUNT;
    sys::memory::setup_heap();
    sys::memory::initialise_system_zones();
    initialise_gfx_zones();

    if( !on_startup() )
    {
        return EXIT_INIT_FAILURE;
    }

    i32 exitcode = app_main();

    on_shutdown();

    sys::log::shutdown();
    return exitcode;
}

bool app::on_startup()
{
    return true;
}

void app::on_shutdown()
{ }

} // fw