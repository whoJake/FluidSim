#include "ApplicationBase.h"
#include "device/fiDevice.h"

PARAM(args_file);

ApplicationBase::ApplicationBase() :
    m_exitFlags(ExitFlagBits::Success)
{ }

ApplicationBase::~ApplicationBase()
{
    app_shutdown();
}

int ApplicationBase::run(int argc, const char* argv[])
{
    if( init_global_log() )
    {
        return ExitFlagBits::InitFailure;
    }

    Param::set_executable(argv[0]);
    Param::add_params(argc-1, &argv[1]);

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
    
    // Properly log exit flags.
    return m_exitFlags;
}

void ApplicationBase::app_shutdown(ExitFlags exitFlags)
{

    m_exitFlags |= exitFlags;

    // Flush and destroy log

    on_app_shutdown();
}

