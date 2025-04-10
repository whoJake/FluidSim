#include "AppStartup.h"

#include "system/log.h"
#include "system/timer.h"
#include <chrono>
#include "system/details/log_console.h"
#include "system/details/basic_log.h"
#include "gfx_core/driver.h"
#include "memory_zone.h"
#include "gfx_fw/program_mgr.h"
#include "gfx_fw/render_interface.h"

#include "cdt/loaders/image_loaders.h"
#include "gfx_core/vulkan/vkdefines.h"
#include "platform/windows/window_glfw.h"

#include "system/memory.h"
#include "system/zone_allocator.h"

static u64 g_frame = 0;

void debug_image();
void debug_triangle();

void debug_vbuffer_start();
void debug_vbuffer_work();
void debug_vbuffer_end();

static bool g_updatefps = true;
static u64 g_fps = 0;
static f32 g_percWaiting = 0.f;

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

	gfx::fw::render_interface::set_target_swapchain_extents(1430, 1079);
	gfx::fw::render_interface::initialise();

	// debug_image();
	// debug_triangle();
	debug_vbuffer_start();

	while( !window.get_should_close() )
	{
		window.process_events();
		debug_vbuffer_work();

		if( g_updatefps  )
		{
			window.set_title(std::format("{} fps | {} waiting", g_fps, g_percWaiting));
			g_updatefps = false;
		}
	}

	debug_vbuffer_end();
	gfx::driver::wait_idle();

	gfx::fw::render_interface::shutdown();
	gfx::program_mgr::shutdown();
	gfx::driver::shutdown();
	
	// AppStartup app;
	// return app.run(argc, argv);
	return 0;
}

void debug_image()
{
	std::unique_ptr<cdt::image> img = cdt::image_loader::from_file_png("C:/Users/Jake/Documents/Projects/UnnamedGame/src/game/game/assets/images/new_years.png");

	gfx::memory_info buf_mem_info = gfx::memory_info::create_as_buffer(img->data(), img->get_size(), gfx::format::R8G8B8A8_SRGB, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::TEXTURE_USAGE_TRANSFER_SRC);
	gfx::buffer buffer = gfx::buffer::create(buf_mem_info);

	gfx::texture_info tex_info;
	tex_info.initialise(img->get_metadata().width, img->get_metadata().height, img->get_metadata().depth, 1);

	gfx::memory_info tex_mem_info = gfx::memory_info::create_as_texture(
		tex_info.get_width() * tex_info.get_height() * tex_info.get_depth(),
		gfx::format::R8G8B8A8_SRGB,
		gfx::MEMORY_TYPE_GPU_ONLY,
		gfx::TEXTURE_USAGE_TRANSFER_DST | gfx::TEXTURE_USAGE_TRANSFER_SRC);

	gfx::texture img_texture = gfx::texture::create(tex_mem_info, tex_info, gfx::RESOURCE_VIEW_2D);

	{
		sys::timer<sys::microseconds> time("Debug image record frame time. {}");

		gfx::fw::render_interface::begin_frame();
		// buffer -> image
		gfx::fw::render_interface::get_list_temp()->texture_memory_barrier(&img_texture, gfx::TEXTURE_LAYOUT_TRANSFER_DST);
		gfx::fw::render_interface::get_list_temp()->copy_to_texture(&buffer, &img_texture);

		//// image -> swapchain
		gfx::texture_view* swap_view = gfx::fw::render_interface::get_active_swapchain_texture_view();
		gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

		gfx::fw::render_interface::get_list_temp()->texture_memory_barrier(&img_texture, gfx::TEXTURE_LAYOUT_TRANSFER_SRC);
		gfx::fw::render_interface::get_list_temp()->texture_memory_barrier(swap_tex, gfx::TEXTURE_LAYOUT_TRANSFER_DST);
		gfx::fw::render_interface::get_list_temp()->copy_to_texture(&img_texture, swap_tex);

		gfx::fw::render_interface::end_frame();
	}

	gfx::driver::wait_idle();
	gfx::texture::destroy(&img_texture);
	gfx::buffer::destroy(&buffer);
}

void debug_triangle()
{
	gfx::program_mgr::load("triangle.fxcp");
	gfx::program* prog = const_cast<gfx::program*>(gfx::program_mgr::find_program(dt::hash_string32("triangle")));

	{
		sys::timer<sys::microseconds> time("Debug triangle record frame time. {}");
		
		gfx::fw::render_interface::begin_frame();

		gfx::texture_view* swap_view = gfx::fw::render_interface::get_active_swapchain_texture_view();
		gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

		// swapchain -> renderable
		gfx::fw::render_interface::get_list_temp()->texture_memory_barrier(swap_tex, gfx::TEXTURE_LAYOUT_COLOR_ATTACHMENT);

		gfx::driver::get_device()->begin_pass(gfx::fw::render_interface::get_list_temp(), prog, 0, swap_view);
		gfx::fw::render_interface::get_list_temp()->draw(3);
		gfx::driver::get_device()->end_pass(gfx::fw::render_interface::get_list_temp());

		gfx::fw::render_interface::end_frame();
	}

	gfx::driver::wait_idle();
}

static gfx::buffer sbuf = { };
static gfx::buffer vbuf = { };
static gfx::program* prog = nullptr;

void debug_vbuffer_start()
{
	gfx::program_mgr::load("basic_vertex_2d.fxcp");
	prog = const_cast<gfx::program*>(gfx::program_mgr::find_program(dt::hash_string32("basic_vertex_2d")));

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

	sbuf = gfx::buffer::create(staging_mem_info);

	gfx::memory_info buf_mem_info = gfx::memory_info::create_as_buffer(
		sizeof(vertex) * 6,
		gfx::format::UNDEFINED,
		gfx::MEMORY_TYPE_GPU_ONLY,
		gfx::BUFFER_USAGE_TRANSFER_DST | gfx::BUFFER_USAGE_VERTEX);

	vbuf = gfx::buffer::create(buf_mem_info);

	gfx::fw::render_interface::begin_frame();

	// staging -> local
	gfx::fw::render_interface::get_list_temp()->copy_buffer(&sbuf, &vbuf);

	// swapchain -> present
	gfx::texture_view* swap_view = gfx::fw::render_interface::get_active_swapchain_texture_view();
	gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

	gfx::fw::render_interface::end_frame();
}

void debug_vbuffer_work()
{
	// Calculate FPS stuff
	{
		static sys::moment lastupdate = sys::now();
		static u64 frames = 0;

		const double seconds_per_update = 1.0;

		++frames;
		{
			sys::moment now = sys::now();
			u64 us_since_last_update = std::chrono::floor<std::chrono::microseconds>(now - lastupdate).count();

			if( us_since_last_update * 0.000001 > seconds_per_update )
			{
				lastupdate = now;

				g_fps = u64_cast(frames / seconds_per_update);
				frames = 0;
				g_updatefps = true;
			}
		}
	}

	static bool flip = false;

	double frame_time = 1.f / g_fps;
	frame_time *= 1000; // ms
	frame_time *= 1000; // us

	sys::moment before = sys::now();
	gfx::fw::render_interface::begin_frame();
	u64 us_since_before = std::chrono::floor<std::chrono::microseconds>(sys::now() - before).count();
	g_percWaiting = (us_since_before / frame_time) * 100.f;

	{
		std::vector<gfx::buffer*> bufs;
		bufs.push_back(&vbuf);

		gfx::texture_view* swap_view = gfx::fw::render_interface::get_active_swapchain_texture_view();
		gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

		// swapchain -> renderable
		gfx::fw::render_interface::get_list_temp()->texture_memory_barrier(swap_tex, gfx::TEXTURE_LAYOUT_COLOR_ATTACHMENT);

		gfx::driver::get_device()->begin_pass(gfx::fw::render_interface::get_list_temp(), prog, 0, swap_view);
		gfx::fw::render_interface::get_list_temp()->bind_vertex_buffers(bufs.data(), u32_cast(bufs.size()));

		u32 offset = flip ? 0 : 3;
		gfx::fw::render_interface::get_list_temp()->draw(3, 1, offset);
		gfx::driver::get_device()->end_pass(gfx::fw::render_interface::get_list_temp());

		flip = !flip;
	}

	gfx::fw::render_interface::end_frame();
}

void debug_vbuffer_end()
{
	gfx::driver::wait_idle();
	gfx::driver::destroy_buffer(&sbuf);
	gfx::driver::destroy_buffer(&vbuf);
}