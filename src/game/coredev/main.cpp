#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"

#include "system/allocator.h"
#include "system/basic_allocator.h"
#include "system/tracked_allocator.h"
#include "dt/pool.h"

#include "system/assert.h"

SYSDECLARE_CHANNEL(main);

int main(int argc, const char* argv[])
{
	sys::allocator::set_main(sys::tracked_allocator::get());
	sys::tracked_allocator::set_allocate_callback([](u64 size, u64 align, void* ret_ptr, sys::memory_zone zone)
		{
			SYSMSG_CHANNEL_PROFILE(main, "Allocating {} bytes in zone {}", size, as_c_str(zone));
		});
	sys::tracked_allocator::set_free_callback([](void* free_ptr, sys::memory_zone zone)
		{
			SYSMSG_CHANNEL_PROFILE(main, "Freeing {:#018x} in zone {}", reinterpret_cast<u64>(free_ptr), as_c_str(zone));
		});

	sys::log::details::logger logger;
	logger.register_target(new sys::log::details::console_target());
	sys::log::initialise(new sys::log::details::basic_log(std::move(logger), sys::log::level::none));

	SYSMSG_CHANNEL_WARN(main, "Hello world!");

	dt::zoned_pool<u8, sys::MEMZONE_GAME> pool(10);
	u8* first = pool.allocate();
	u8* second = pool.allocate();
	pool.free(first);
	u8* third = pool.allocate();

	pool.free(second);
	pool.free(third);

	std::vector<u8*> fella;
	for( u32 i = 0; i < 10; i++ )
	{
		fella.push_back(pool.allocate());
	}

	for( u8* guy : fella )
	{
		pool.free(guy);
	}
}