#include "AppStartup.h"

// static u64 g_frame = 0;
// 
// void debug_image();
// void debug_triangle();
// 
// void debug_vbuffer_start();
// void debug_vbuffer_work();
// void debug_vbuffer_end();
// 
// void debug_ui();
// 
// static bool g_updatefps = true;

int main(int argc, const char* argv[])
{
	/*
	sys::zone_allocator::max_zones = MEMZONE_SYSTEM_COUNT + MEMZONE_GFX_COUNT;
	sys::memory::setup_heap();
	sys::memory::initialise_system_zones();

	std::vector<const char*> args(argc - 1);
	for( i32 i = 1; i < argc; i++ )
		args[i - 1] = argv[i];
	sys::param::init(args);
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
		fw::cursor_lock_state::none,
		[](Event& e)
		{
			EventDispatcher dispatch(e);
			dispatch.dispatch<WindowResizeEvent>([](WindowResizeEvent& ev) -> bool
				{
					gfx::fw::render_interface::set_target_swapchain_extents(ev.get_width(), ev.get_height());
					return true;
				});
		}
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
	// debug_vbuffer_start();
	debug_ui();
	f32 frame_time_ms = 0.f;
	f32 wait_frame_ms = 0.f;

	f32 rolling_ms = 0.f;

	while( !window.get_should_close() )
	{
		static sys::moment before_frame = sys::now();
		before_frame = sys::now();

		window.process_events();
		{
			sys::moment before_wait = sys::now();
			// gfx::fw::render_interface::wait_for_frame();
			sys::moment after_wait = sys::now();
			
			f32 cur_wait_frame_ms = std::chrono::floor<std::chrono::microseconds>(after_wait - before_wait).count() / 1000.f;
			wait_frame_ms = (wait_frame_ms * 0.9f) + (cur_wait_frame_ms * 0.1f);
		}

		// debug_vbuffer_work();

		sys::moment after_frame = sys::now();

		f32 cur_frame_time_ms = std::chrono::floor<std::chrono::microseconds>(after_frame - before_frame).count() / 1000.f;
		frame_time_ms = (frame_time_ms * 0.9f) + (cur_frame_time_ms * 0.1f);

		rolling_ms += cur_frame_time_ms;
		if( rolling_ms >= 750.f )
		{
			rolling_ms = 0.f;
			window.set_title(std::format("Frame time: {:.3f}ms | Wait time: {:.3f}ms | Percentage: {}% | FPS: {:.2f}", frame_time_ms, wait_frame_ms, u32_cast((wait_frame_ms / frame_time_ms) * 100), 1000.f / frame_time_ms));
		}
	}

	// debug_vbuffer_end();
	gfx::driver::wait_idle();

	gfx::fw::render_interface::shutdown();
	gfx::program_mgr::shutdown();
	gfx::driver::shutdown();
	*/

	AppStartup app;
	return app.run(argc, argv);
	// return 0;
}

/*
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
	gfx::fw::render_interface::end_frame();
}

void debug_vbuffer_work()
{
	static bool flip = false;
	gfx::fw::render_interface::begin_frame(false);
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

void debug_ui()
{
	struct position
	{
		f32 x, y;
	};

	struct uv
	{
		f32 u, v;
	};

	struct vertex
	{
		position pos;
		uv texcoord0;
	};

	vertex vertices[] =
	{
		{ { -0.75f, -0.75f }, { -0.2f, -0.2f } }, // 0
		{ {  0.75f, -0.75f }, { 1.2f, -0.2f } }, // 1
		{ { -0.75f,  0.75f }, { -0.2f, 1.2f } }, // 2

		{ {  0.75f,  0.75f }, { 1.2f, 1.2f } }, // 3
		{ { -0.75f,  0.75f }, { -0.2f, 1.2f } }, // 4
		{ {  0.75f, -0.75f }, { 1.2f, -0.2f } }, // 5
	};

	gfx::memory_info staging_mem_info = gfx::memory_info::create_as_buffer(
		vertices,
		sizeof(vertex) * 6,
		gfx::format::UNDEFINED,
		gfx::MEMORY_TYPE_CPU_VISIBLE,
		gfx::BUFFER_USAGE_TRANSFER_SRC);

	gfx::buffer staging_buffer = gfx::buffer::create(staging_mem_info);

	gfx::memory_info buf_mem_info = gfx::memory_info::create_as_buffer(
		sizeof(vertex) * 6,
		gfx::format::UNDEFINED,
		gfx::MEMORY_TYPE_GPU_ONLY,
		gfx::BUFFER_USAGE_TRANSFER_DST | gfx::BUFFER_USAGE_VERTEX);

	gfx::buffer vertex_buffer = gfx::buffer::create(buf_mem_info);

	std::unique_ptr<cdt::image> image = cdt::image_loader::from_file_png("C:/Users/Jake/Documents/Projects/UnnamedGame/src/game/game/assets/images/new_years.png");

	gfx::memory_info image_buf_mem_info = gfx::memory_info::create_as_buffer(image->data(), image->get_size(), gfx::format::R8G8B8A8_SRGB, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::TEXTURE_USAGE_TRANSFER_SRC);
	gfx::buffer image_staging_buffer = gfx::buffer::create(image_buf_mem_info);

	gfx::texture_info texture_info{ };
	texture_info.initialise(image->get_metadata().width, image->get_metadata().height, image->get_metadata().depth, 1);

	gfx::memory_info tex_mem_info = gfx::memory_info::create_as_texture(
		texture_info.get_width() * texture_info.get_height() * texture_info.get_depth(),
		gfx::format::R8G8B8A8_SRGB,
		gfx::MEMORY_TYPE_GPU_ONLY,
		gfx::TEXTURE_USAGE_TRANSFER_DST | gfx::TEXTURE_USAGE_SAMPLED);

	gfx::texture image_texture = gfx::texture::create(tex_mem_info, texture_info, gfx::RESOURCE_VIEW_2D);

	{
		// Create our initial buffers
		gfx::fw::render_interface::begin_frame();

		RI_GraphicsContext.copy_buffer(&staging_buffer, &vertex_buffer);
		RI_GraphicsContext.texture_layout_transition(&image_texture, gfx::TEXTURE_LAYOUT_TRANSFER_DST);
		RI_GraphicsContext.copy_buffer(&image_staging_buffer, &image_texture);
		RI_GraphicsContext.texture_layout_transition(&image_texture, gfx::TEXTURE_LAYOUT_SHADER_READONLY);

		gfx::fw::render_interface::end_frame();
	}

	gfx::program_mgr::load("basic_ui.fxcp");
	gfx::program* program = const_cast<gfx::program*>(gfx::program_mgr::find_program(dt::hash_string32("basic_ui")));

	gfx::descriptor_pool desc_pool = gfx::descriptor_pool();
	desc_pool.initialise(program->get_pass(0).get_descriptor_table(gfx::DESCRIPTOR_TABLE_PER_FRAME), 1);

	gfx::descriptor_table* table = desc_pool.allocate();

	gfx::texture_view image_view = image_texture.create_view(gfx::format::R8G8B8A8_SRGB, gfx::RESOURCE_VIEW_2D, { 0, 1, 0, 1 });
	gfx::texture_sampler image_sampler = image_view.create_sampler();

	table->set_image(dt::hash_string32("in_sampler0"), &image_sampler);
	gfx::driver::get_device()->write_descriptor_table(table);

	{
		// Draw and render
		gfx::fw::render_interface::begin_frame();

		gfx::texture_view* swap_view = gfx::fw::render_interface::get_active_swapchain_texture_view();
		gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());

		gfx::texture_attachment attachment
		{
			.view = swap_view,
			.load = gfx::LOAD_OP_CLEAR,
			.store = gfx::STORE_OP_STORE,
		};

		// swapchain -> renderable
		RI_GraphicsContext.texture_layout_transition(swap_tex, gfx::TEXTURE_LAYOUT_COLOR_ATTACHMENT);

		RI_GraphicsContext.begin_rendering({ attachment }, nullptr);
		RI_GraphicsContext.bind_program(program, 0);

		RI_GraphicsContext.set_viewport(0.f, 0.f, f32_cast(image_texture.get_width()), f32_cast(image_texture.get_height()), 0.f, 1.f);
		RI_GraphicsContext.set_scissor(0, 0, image_texture.get_width(), image_texture.get_height());

		RI_GraphicsContext.bind_vertex_buffers({ &vertex_buffer });
		RI_GraphicsContext.bind_descriptor_table(&program->get_pass(0), table, gfx::DESCRIPTOR_TABLE_PER_FRAME);

		RI_GraphicsContext.draw(6);
		RI_GraphicsContext.end_rendering();

		gfx::fw::render_interface::end_frame();
	}

	gfx::driver::wait_idle();

	gfx::driver::get_device()->destroy_descriptor_pool(&desc_pool);
	gfx::texture_sampler::destroy(&image_sampler);
	gfx::texture_view::destroy(&image_view);
	gfx::texture::destroy(&image_texture);
	gfx::buffer::destroy(&image_staging_buffer);
	gfx::buffer::destroy(&staging_buffer);
	gfx::buffer::destroy(&vertex_buffer);
}

*/