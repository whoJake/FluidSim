#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"

#include "system/allocator.h"
#include "system/basic_allocator.h"
#include "system/zone_allocator.h"

#include "system/assert.h"
#include "dt/hash_string.h"

SYSDECLARE_CHANNEL(main);

int main(int argc, const char* argv[])
{
	sys::allocator::set_underlying_allocator(sys::zone_allocator::get());

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

void* operator new(u64 size)
{
	return sys::allocator::allocate(size, sys::allocator::default_align);
}

void* operator new(u64 size, u64 align)
{
	return sys::allocator::allocate(size, align);
}

void operator delete(void* ptr)
{
	return sys::allocator::free(ptr, 0);
}

void operator delete(void* ptr, u64 size)
{
	return sys::allocator::free(ptr, size);
}