#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"

#include "system/allocator.h"
#include "system/basic_allocator.h"
#include "system/tracked_allocator.h"

#include "system/assert.h"
#include "dt/hash_string.h"

SYSDECLARE_CHANNEL(main);

int main(int argc, const char* argv[])
{
	sys::allocator::set_main(sys::tracked_allocator::get());
	sys::tracked_allocator::set_allocate_callback([](u64 size, u64 align, void* ret_ptr, sys::memory_zone zone)
		{
			SYSMSG_CHANNEL_PROFILE(main, "Allocating {} bytes in zone {}", size, SYSZONE_NAME(zone));
		});
	sys::tracked_allocator::set_free_callback([](void* free_ptr, sys::memory_zone zone)
		{
			SYSMSG_CHANNEL_PROFILE(main, "Freeing {:#018x} in zone {}", reinterpret_cast<u64>(free_ptr), SYSZONE_NAME(zone));
		});

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