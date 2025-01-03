#include "AppStartup.h"

#include "system/log.h"
#include "system/details/log_console.h"
#include "system/details/basic_log.h"
#include "gfx_core/Driver.h"

#include "cdt/loaders/image_loaders.h"
#include "vulkan/vkdefines.h"
#include "platform/windows/window_glfw.h"

int main(int argc, const char* argv[])
{
	sys::log::details::logger logger;
	logger.register_target(new sys::log::details::console_target());
	sys::log::initialise(new sys::log::details::basic_log(std::move(logger), sys::log::level::none));

	fw::window::state state
	{
		"GFX_CORE Example",
		fw::window::mode::windowed,
		true,
		true,
		{ 0, 0 },
		{ 1430, 1079 },
	};

	fw::window_glfw window(state);

	u32 result = gfx::Driver::initialise(
		gfx::DriverMode::vulkan,
		std::bind(
			&fw::window::create_vulkan_surface,
			&window,
			std::placeholders::_1,
			std::placeholders::_2));

	gfx::device* device = gfx::Driver::get_device();

	gfx::Driver::get_device()->dump_info();

	gfx::surface_capabilities sc = device->get_surface_capabilities();

	gfx::texture_info tInfo{ };
	tInfo.initialise(
		cdt::image_format::R8G8B8A8_SRGB,
		gfx::texture_usage_flag_bits::texture_transfer_dst | gfx::texture_usage_flag_bits::texture_color,
		window.get_extent().x,
		window.get_extent().y,
		1);

	gfx::swapchain swapchain = device->create_swapchain(nullptr, tInfo, gfx::present_mode::immediate);
	u32 swapIndex = swapchain.aquire_next_image();
	gfx::texture* swapTexture = swapchain.get_image(swapIndex);
	swapchain.wait_for_index(swapIndex);

	gfx::fence swapFence = device->create_fence(true);
	swapFence.reset();

	std::unique_ptr<cdt::image> img = cdt::image_loader::from_file_png("assets/images/new_years.png");

	gfx::buffer buffer = device->create_buffer(img->get_size(), gfx::buffer_usage_bits::buffer_transfer_src, gfx::memory_type::cpu_accessible, true);

	gfx::texture_info texInfo;
	texInfo.initialise(img->get_metadata().format, gfx::texture_sampled | gfx::texture_transfer_src | gfx::texture_transfer_dst, img->get_metadata().width, img->get_metadata().height, img->get_metadata().depth);
	gfx::texture texture = device->create_texture(texInfo, gfx::resource_view_type::texture_2d, gfx::memory_type::gpu_only, false);

	memcpy(buffer.get_memory_info().mapped, img->data(), img->get_size());

	gfx::graphics_command_list swapList = device->allocate_graphics_command_list();
	swapList.begin();

	// buffer -> image
	swapList.texture_memory_barrier(&texture, gfx::texture_layout::transfer_dst);
	swapList.copy_to_texture(&buffer, &texture);

	// image -> swapchain
	swapList.texture_memory_barrier(&texture, gfx::texture_layout::transfer_src);
	swapList.texture_memory_barrier(swapTexture, gfx::texture_layout::transfer_dst);
	swapList.copy_to_texture(&texture, swapTexture);

	swapList.texture_memory_barrier(swapTexture, gfx::texture_layout::present);
	swapList.end();
	swapList.submit(&swapFence);

	swapFence.wait();

	device->free_texture(&texture);
	device->free_buffer(&buffer);
	device->free_command_list(&swapList);
	device->free_fence(&swapFence);

	swapchain.present(swapIndex);

	while( 1 )
	{
		window.process_events();
	}

	device->free_swapchain(&swapchain);

	gfx::Driver::shutdown();

	AppStartup app;
	return app.run(argc, argv);
}