#include "AppStartup.h"

#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"
#include "gfx_core/Driver.h"

int main(int argc, const char* argv[])
{
	sys::log::details::logger logger;
	logger.register_target(new sys::log::details::console_target());
	sys::log::initialise(new sys::log::details::basic_log(std::move(logger), sys::log::level::none));

	u32 result = gfx::Driver::initialise(gfx::DriverMode::vulkan);

	AppStartup app;
	return app.run(argc, argv);
}