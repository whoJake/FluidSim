#include "ApplicationBase.h"
#include "device/fiDevice.h"

PARAM(args_file);

ApplicationBase::ApplicationBase() :
    m_exitFlags(ExitFlagBits::Success),
    m_running(false)
{ }

ApplicationBase::~ApplicationBase()
{
    app_shutdown();
}

int ApplicationBase::run(int argc, const char* argv[])
{
    m_running = true;

    if( init_global_log() )
    {
        return ExitFlagBits::InitFailure;
    }

    Param::set_executable(argv[0]);
    Param::add_params(argc-1, &argv[1]);

    // Read args file and add it to the list of args we have.
    if( Param_args_file.get() )
    {
        const char* filename = Param_args_file.value();
        fiDevice device;
        device.open(filename);
        std::vector<uint8_t> buffer;
        while( device.read_line(&buffer) )
        {
            char* persistentStr = new char[buffer.size()+1];
            memcpy(persistentStr, buffer.data(), buffer.size());
            persistentStr[buffer.size()] = '\0';
            Param::add_params(1, (const char**)&persistentStr);
        }
    }

    ExitFlags exitFlags;
    exitFlags = app_main();
    
    on_app_shutdown();

    // Properly log exit flags.
    return m_exitFlags;
}

void ApplicationBase::app_shutdown(ExitFlags exitFlags)
{
    if( !m_running )
    {
        return;
    }

    m_exitFlags |= exitFlags;
    // Flush and destroy log

    m_running = false;
    on_app_shutdown();
}

