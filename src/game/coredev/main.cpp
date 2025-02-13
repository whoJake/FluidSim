#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"

#include "system/allocator.h"
#include "system/basic_allocator.h"
#include "system/tracked_allocator.h"
#include "dt/pool.h"

#include "system/assert.h"
#include "dt/vector.h"
#include "dt/bitset.h"
#include "dt/unique_ptr.h"

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

	struct S
	{
		S()
		{
			SYSMSG_DEBUG("Constructing.");
		}

		S(const S&)
		{
			SYSMSG_DEBUG("Copy Constructing.");
		}

		S(S&&)
		{
			SYSMSG_DEBUG("Move Constructing.");
		}

		S& operator=(const S&)
		{
			SYSMSG_DEBUG("Copy.");
			return *this;
		}

		S& operator=(S&&)
		{
			SYSMSG_DEBUG("Move.");
			return *this;
		}

		~S()
		{
			SYSMSG_DEBUG("Destructing.");
		}
	};

	{
		dt::unique_ptr<u32> ptr = dt::make_unique<u32>(5);
		SYSMSG_DEBUG("{}", *ptr);
		ptr = dt::make_unique<u32>(6);
		SYSMSG_DEBUG("{}", *ptr);
		ptr.reset();

		dt::bitset<u32> set;
		set.set(151, true);
		bool a = set.is_set(32);
		set.set(0, true);
		set.set(1, true);
		bool b = set.is_set(0);
		bool c = set.is_set(1);
		bool d = set.is_set(2);

	}
}