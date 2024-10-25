#include "AppStartup.h"

#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"
#include "gfx_core/Driver.h"

#include "cdt/loaders/image_loaders.h"
#include "vulkan/vkdefines.h"

int main(int argc, const char* argv[])
{
	sys::log::details::logger logger;
	logger.register_target(new sys::log::details::console_target());
	sys::log::initialise(new sys::log::details::basic_log(std::move(logger), sys::log::level::none));

	u32 result = gfx::Driver::initialise(gfx::DriverMode::vulkan);
	gfx::device* device = gfx::Driver::get_device();

	gfx::Driver::get_device()->dump_info();


	std::unique_ptr<cdt::image> img = cdt::image_loader::from_file_png("assets/images/test.png");

	gfx::buffer buffer = device->create_buffer(img->get_size(), gfx::buffer_usage_bits::buffer_transfer_src, gfx::memory_type::cpu_accessible, true);

	gfx::texture_info texInfo(img->get_metadata().format, gfx::texture_usage_flag_bits::sampled | gfx::texture_usage_flag_bits::texture_transfer_dst, img->get_metadata().width, img->get_metadata().height, img->get_metadata().depth);
	gfx::texture texture = device->create_texture(texInfo, gfx::resource_view_type::texture_2d, gfx::memory_type::gpu_only, false);

	memcpy(buffer.get_memory_info().mapped, img->data(), img->get_size());

	device->free_texture(&texture);
	device->free_buffer(&buffer);

	gfx::Driver::shutdown();

	AppStartup app;
	return app.run(argc, argv);
}