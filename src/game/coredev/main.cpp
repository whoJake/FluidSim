#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"

#include "system/assert.h"
#include "dt/hash_string.h"
#include "system/memory.h"

SYSDECLARE_CHANNEL(main);

int main(int argc, const char* argv[])
{
	sys::memory::setup_heap();
	sys::memory::initialise_system_zones();

	sys::log::details::logger logger;
	logger.register_target(new sys::log::details::console_target());
	sys::log::initialise(new sys::log::details::basic_log(std::move(logger), sys::log::level::none));

	SYSMSG_CHANNEL_WARN(main, "Hello world!");

	dt::hash_string32 str1("Testing0");
	dt::hash_string32 str2("Testing1");
	dt::hash_string32 str3("Testing2");
	dt::hash_string32 str4("Testing3");
	dt::hash_string32 str5("Testing4");
	dt::hash_string32 str6("Testing5");

	SYSMSG_CHANNEL_INFO(main, "{}: {}", str1.get_hash(), str1.try_get_str());
	SYSMSG_CHANNEL_INFO(main, "{}: {}", str2.get_hash(), str2.try_get_str());
	SYSMSG_CHANNEL_INFO(main, "{}: {}", str3.get_hash(), str3.try_get_str());
	SYSMSG_CHANNEL_INFO(main, "{}: {}", str4.get_hash(), str4.try_get_str());
	SYSMSG_CHANNEL_INFO(main, "{}: {}", str5.get_hash(), str5.try_get_str());
	SYSMSG_CHANNEL_INFO(main, "{}: {}", str6.get_hash(), str6.try_get_str());

}