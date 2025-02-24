#include "AppStartup.h"

#include "system/log.h"
#include "system/timer.h"
#include <chrono>
#include "system/details/log_console.h"
#include "system/details/basic_log.h"
#include "gfx_core/Driver.h"
#include "gfx_core/loaders/loaders.h"

#include "cdt/loaders/image_loaders.h"
#include "vulkan/vkdefines.h"
#include "platform/windows/window_glfw.h"

static u64 g_frame = 0;

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
		gfx::texture_usage_flag_bits::TEXTURE_USAGE_TRANSFER_SRC | gfx::TEXTURE_USAGE_TRANSFER_DST | gfx::TEXTURE_USAGE_COLOR,
		window.get_extent().x,
		window.get_extent().y,
		1);

	gfx::swapchain swapchain = device->create_swapchain(nullptr, tInfo, gfx::present_mode::PRESENT_MODE_IMMEDIATE);
	u32 swapIndex = swapchain.aquire_next_image();
	gfx::texture* swapTexture = swapchain.get_image(swapIndex);
	swapchain.wait_on_present(swapIndex);

	std::unique_ptr<cdt::image> img = cdt::image_loader::from_file_png("assets/images/new_years.png");

	gfx::buffer buffer = device->create_buffer(img->get_size(), gfx::buffer_usage_bits::BUFFER_USAGE_TRANSFER_SRC, gfx::memory_type::cpu_accessible, true);

	gfx::texture_info texInfo;
	texInfo.initialise(img->get_metadata().format, gfx::TEXTURE_USAGE_SAMPLED | gfx::TEXTURE_USAGE_TRANSFER_SRC | gfx::TEXTURE_USAGE_TRANSFER_DST, img->get_metadata().width, img->get_metadata().height, img->get_metadata().depth);
	gfx::texture texture = device->create_texture(texInfo, gfx::resource_view_type::texture_2d, gfx::memory_type::gpu_only, false);

	memcpy(buffer.get_memory_info().mapped, img->data(), img->get_size());

	{
		gfx::fence swapFence = device->create_fence(false);
		gfx::dependency dep = device->create_dependency("SUBMIT_DEPENDENCY");

		sys::timer<sys::microseconds> time("Record command list. {}");
		gfx::graphics_command_list swapList = device->allocate_graphics_command_list();
		swapList.set_signal_dependency(&dep);

		swapList.begin();

		// buffer -> image
		swapList.texture_memory_barrier(&texture, gfx::texture_layout::TEXTURE_LAYOUT_TRANSFER_DST);
		swapList.copy_to_texture(&buffer, &texture);

		// image -> swapchain
		swapList.texture_memory_barrier(&texture, gfx::texture_layout::TEXTURE_LAYOUT_TRANSFER_SRC);
		swapList.texture_memory_barrier(swapTexture, gfx::texture_layout::TEXTURE_LAYOUT_TRANSFER_DST);
		swapList.copy_to_texture(&texture, swapTexture);

		swapList.texture_memory_barrier(swapTexture, gfx::texture_layout::TEXTURE_LAYOUT_PRESENT);
		swapList.end();
		swapList.submit(&swapFence);

		swapchain.present(swapIndex, { &dep });

		swapFence.wait();
		device->free_command_list(&swapList);
		device->free_dependency(&dep);
		device->free_fence(&swapFence);

	}

	device->free_texture(&texture);
	device->free_buffer(&buffer);


	gfx::program prog{ };
	gfx::loaders::load("../shaderdev/compiled/triangle.fxcp", &prog);

	const_cast<gfx::pass*>(&prog.get_pass(0))->m_pLayoutImpl = gfx::Driver::get_device()->create_shader_pass_layout_impl(const_cast<gfx::pass*>(&prog.get_pass(0)));
	const_cast<gfx::pass*>(&prog.get_pass(0))->m_pImpl = gfx::Driver::get_device()->create_shader_pass_impl(&prog, 0);

	sys::moment lastupdate = sys::now();
	u64 frames = 0;

	const double seconds_per_update = 1.0;
	while( 1 )
	{
		window.process_events();
		++frames;

		sys::moment now = sys::now();
		u64 us_since_last_update = std::chrono::floor<std::chrono::microseconds>(now - lastupdate).count();

		if( us_since_last_update * 0.000001 > seconds_per_update  )
		{
			lastupdate = now;

			u64 fps = u64_cast(frames / seconds_per_update);
			frames = 0;

			window.set_title(std::format("{} fps", fps));
		}

		g_frame++;
	}

	device->free_swapchain(&swapchain);

	gfx::Driver::shutdown();

	AppStartup app;
	return app.run(argc, argv);
}