#include "AppStartup.h"

#include "system/log.h"
#include "system/timer.h"
#include <chrono>
#include "system/details/log_console.h"
#include "system/details/basic_log.h"
#include "gfx_core/driver.h"
#include "memory_zone.h"
#include "gfx_fw/program_mgr.h"

#include "cdt/loaders/image_loaders.h"
#include "gfx_core/vulkan/vkdefines.h"
#include "platform/windows/window_glfw.h"

#include "system/memory.h"
#include "system/zone_allocator.h"

static u64 g_frame = 0;

void debug_image(gfx::texture_view* swap_view, gfx::swapchain* swapchain, u32 swapIndex);
void debug_triangle(gfx::texture_view* swap_view, gfx::swapchain* swapchain, u32 swapIndex);
void debug_vbuffer(gfx::texture_view* swap_view, gfx::swapchain* swapchain, u32 swapIndex);

int main(int argc, const char* argv[])
{
	sys::zone_allocator::max_zones = MEMZONE_SYSTEM_COUNT + MEMZONE_GFX_COUNT;
	sys::memory::setup_heap();
	sys::memory::initialise_system_zones();
	initialise_gfx_zones();

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

	u32 result = gfx::driver::initialise(
		gfx::DRIVER_MODE_VULKAN,
		std::bind(
			&fw::window::create_vulkan_surface,
			&window,
			std::placeholders::_1,
			std::placeholders::_2));

	gfx::program_mgr::initialise("C:\\Users\\Jake\\Documents\\Projects\\UnnamedGame\\src\\game\\shaderdev\\compiled\\");

	gfx::device* device = gfx::driver::get_device();
	gfx::driver::get_device()->dump_info();

	gfx::surface_capabilities sc = device->get_surface_capabilities();

	gfx::texture_info tInfo{ };
	tInfo.initialise(
		window.get_extent().x,
		window.get_extent().y,
		1,
		1);

	gfx::swapchain swapchain = device->create_swapchain(
		nullptr,
		tInfo,
		gfx::TEXTURE_USAGE_TRANSFER_SRC | gfx::TEXTURE_USAGE_TRANSFER_DST | gfx::TEXTURE_USAGE_COLOR,
		gfx::format::R8G8B8A8_SRGB,
		gfx::present_mode::PRESENT_MODE_IMMEDIATE);

	u32 swapIndex = swapchain.aquire_next_image();
	gfx::texture* swapTexture = swapchain.get_image(swapIndex);
	gfx::texture_view swap_view = swapTexture->create_view(gfx::format::R8G8B8A8_SRGB, gfx::RESOURCE_VIEW_2D, { 0, 1, 0, 1 });
	swapchain.wait_on_present(swapIndex);

	debug_image(&swap_view, &swapchain, swapIndex);
	// debug_triangle(&swap_view, &swapchain, swapIndex);
	// debug_vbuffer(&swap_view, &swapchain, swapIndex);

	sys::moment lastupdate = sys::now();
	u64 frames = 0;

	const double seconds_per_update = 1.0;
	while( !window.get_should_close() )
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

	gfx::texture_view::destroy(&swap_view);
	device->free_swapchain(&swapchain);

	gfx::program_mgr::shutdown();
	gfx::driver::shutdown();
	
	// AppStartup app;
	// return app.run(argc, argv);
	return 0;
}

void debug_image(gfx::texture_view* swap_view, gfx::swapchain* swapchain, u32 swapIndex)
{
	gfx::device* device = gfx::driver::get_device();

	std::unique_ptr<cdt::image> img = cdt::image_loader::from_file_png("assets/images/new_years.png");

	gfx::memory_info buf_mem_info = gfx::memory_info::create_as_buffer(img->data(), img->get_size(), gfx::format::R8G8B8A8_SRGB, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::TEXTURE_USAGE_TRANSFER_SRC);
	gfx::buffer buffer = gfx::buffer::create(buf_mem_info);

	gfx::texture_info tex_info;
	tex_info.initialise(img->get_metadata().width, img->get_metadata().height, img->get_metadata().depth, 1);

	gfx::memory_info tex_mem_info = gfx::memory_info::create_as_texture(
		tex_info.get_width() * tex_info.get_height() * tex_info.get_depth(),
		gfx::format::R8G8B8A8_SRGB,
		gfx::MEMORY_TYPE_GPU_ONLY,
		gfx::TEXTURE_USAGE_TRANSFER_DST | gfx::TEXTURE_USAGE_TRANSFER_SRC);
	gfx::texture img_texture = gfx::texture::create(tex_mem_info, tex_info, gfx::texture_layout::TEXTURE_LAYOUT_GENERAL, gfx::RESOURCE_VIEW_2D);

	gfx::fence swapFence = device->create_fence(false);
	gfx::dependency dep = device->create_dependency("SUBMIT_DEPENDENCY");

	gfx::graphics_command_list swapList = device->allocate_graphics_command_list();

	{
		sys::timer<sys::microseconds> time("Record command list. {}");
		swapList.set_signal_dependency(&dep);
		swapList.begin();

		// buffer -> image
		swapList.texture_memory_barrier(&img_texture, gfx::texture_layout::TEXTURE_LAYOUT_TRANSFER_DST);
		swapList.copy_to_texture(&buffer, &img_texture);

		//// image -> swapchain
		gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

		swapList.texture_memory_barrier(&img_texture, gfx::texture_layout::TEXTURE_LAYOUT_TRANSFER_SRC);
		swapList.texture_memory_barrier(swap_tex, gfx::texture_layout::TEXTURE_LAYOUT_TRANSFER_DST);
		swapList.copy_to_texture(&img_texture, swap_tex);

		swapList.texture_memory_barrier(swap_tex, gfx::texture_layout::TEXTURE_LAYOUT_PRESENT);
		swapList.end();
	}

	swapList.submit(&swapFence);

	swapchain->present(swapIndex, { &dep });

	swapFence.wait();
	device->free_command_list(&swapList);
	device->free_dependency(&dep);
	device->free_fence(&swapFence);

	device->free_texture(&img_texture);
	device->free_buffer(&buffer);
}

void debug_triangle(gfx::texture_view* swap_view, gfx::swapchain* swapchain, u32 swapIndex)
{
	gfx::device* device = gfx::driver::get_device();

	gfx::program_mgr::load("triangle.fxcp");
	gfx::program* prog = const_cast<gfx::program*>(gfx::program_mgr::find_program(dt::hash_string32("triangle")));

	gfx::fence swapFence = device->create_fence(false);
	gfx::dependency dep = device->create_dependency("SUBMIT_DEPENDENCY");

	gfx::graphics_command_list swapList = device->allocate_graphics_command_list();
	{
		sys::timer<sys::microseconds> time("Record command list. {}");
		swapList.set_signal_dependency(&dep);

		swapList.begin();
		gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

		// swapchain -> renderable
		swapList.texture_memory_barrier(swap_tex, gfx::texture_layout::TEXTURE_LAYOUT_COLOR_ATTACHMENT);

		gfx::driver::get_device()->begin_pass(&swapList, prog, 0, swap_view);
		swapList.draw(3);
		gfx::driver::get_device()->end_pass(&swapList);

		swapList.texture_memory_barrier(swap_tex, gfx::texture_layout::TEXTURE_LAYOUT_PRESENT);
		swapList.end();
	}
	swapList.submit(&swapFence);

	swapchain->present(swapIndex, { &dep });

	swapFence.wait();
	device->free_command_list(&swapList);
	device->free_dependency(&dep);
	device->free_fence(&swapFence);
}

void debug_vbuffer(gfx::texture_view* swap_view, gfx::swapchain* swapchain, u32 swapIndex)
{
	gfx::device* device = gfx::driver::get_device();
	gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

	gfx::program_mgr::load("basic_vertex_2d.fxcp");
	gfx::program* prog = const_cast<gfx::program*>(gfx::program_mgr::find_program(dt::hash_string32("basic_vertex_2d")));

	struct position
	{
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	struct color
	{
		f32 r;
		f32 g;
		f32 b;
		f32 a;
	};

	struct vertex
	{
		position pos;
		color col;
	};

	position offset{ 0.1f, 0.1f, 0.0f };

	vertex vertices[] =
	{
		{ { -0.5f - 0.03, -0.5f - 0.03, 0.0f }, { 1.f, 0.f, 0.f } }, // 0
		{ {  0.5f - 0.03, -0.5f - 0.03, 0.0f }, { 0.f, 1.f, 0.f } }, // 1
		{ { -0.5f - 0.03,  0.5f - 0.03, 0.0f }, { 0.f, 0.f, 1.f } }, // 2

		{ {  0.5f + 0.03,  0.5f + 0.03, 0.0f }, { 1.f, 1.f, 0.f } }, // 3
		{ { -0.5f + 0.03,  0.5f + 0.03, 0.0f }, { 0.f, 0.f, 1.f } }, // 4
		{ {  0.5f + 0.03, -0.5f + 0.03, 0.0f }, { 0.f, 1.f, 0.f } }, // 5
	};

	gfx::memory_info staging_mem_info = gfx::memory_info::create_as_buffer(
		vertices,
		sizeof(vertex) * 6,
		gfx::format::UNDEFINED,
		gfx::MEMORY_TYPE_CPU_VISIBLE,
		gfx::BUFFER_USAGE_TRANSFER_SRC);

	gfx::buffer staging_buf = gfx::buffer::create(staging_mem_info);

	gfx::memory_info buf_mem_info = gfx::memory_info::create_as_buffer(
		sizeof(vertex) * 6,
		gfx::format::UNDEFINED,
		gfx::MEMORY_TYPE_GPU_ONLY,
		gfx::BUFFER_USAGE_TRANSFER_DST | gfx::BUFFER_USAGE_VERTEX);

	gfx::buffer buffer = gfx::buffer::create(buf_mem_info);

	gfx::fence swapFence = device->create_fence(false);
	gfx::dependency dep = device->create_dependency("SUBMIT_DEPENDENCY");

	gfx::graphics_command_list swapList = device->allocate_graphics_command_list();
	{
		sys::timer<sys::microseconds> time("Record command list. {}");
		swapList.set_signal_dependency(&dep);

		swapList.begin();

		// swapchain -> renderable
		swapList.texture_memory_barrier(swap_tex, gfx::texture_layout::TEXTURE_LAYOUT_COLOR_ATTACHMENT);

		// staging -> local
		swapList.copy_buffer(&staging_buf, &buffer);

		std::vector<gfx::buffer*> bufs;
		bufs.push_back(&buffer);

		device->begin_pass(&swapList, prog, 0, swap_view);

		swapList.bind_vertex_buffers(bufs.data(), u32_cast(bufs.size()));
		swapList.draw(6);
		device->end_pass(&swapList);

		swapList.texture_memory_barrier(swap_tex, gfx::texture_layout::TEXTURE_LAYOUT_PRESENT);
		swapList.end();
	}
	swapList.submit(&swapFence);

	swapchain->present(swapIndex, { &dep });

	swapFence.wait();
	device->free_command_list(&swapList);
	device->free_dependency(&dep);
	device->free_fence(&swapFence);

	device->free_buffer(&staging_buf);
	device->free_buffer(&buffer);
}