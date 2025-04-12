#include "render_interface.h"

#include "../gfx_core/driver.h"
#include "../gfx_core/texture.h"

namespace gfx
{
namespace fw
{

void render_interface::initialise()
{
    screen_capabilities capabilities = driver::query_screen_capabilities();
    GFX_ASSERT(GFX_RI_FRAMES_IN_FLIGHT >= capabilities.min_image_count, "Compiled frames in flight {} is smaller than the min allowed count {}.", GFX_RI_FRAMES_IN_FLIGHT, capabilities.min_image_count);
    GFX_ASSERT(GFX_RI_FRAMES_IN_FLIGHT <= capabilities.max_image_count, "Compiled frames in flight {} is larger than the max allowed count {}.", GFX_RI_FRAMES_IN_FLIGHT, capabilities.max_image_count);
    
    // Creates the initial swapchain.
    u32 swapchain_width = sm_targetSwapchainWidth;
    u32 swapchain_height = sm_targetSwapchainHeight;

    if( swapchain_width < capabilities.min_width )
        swapchain_width = capabilities.min_width;
    if( swapchain_width > capabilities.max_width )
        swapchain_width = capabilities.max_width;

    if( swapchain_height < capabilities.min_height )
        swapchain_height = capabilities.min_height;
    if( swapchain_height > capabilities.max_height )
        swapchain_height = capabilities.max_height;

    texture_info tex_info(swapchain_width, swapchain_height, 1, 1);
    sm_swapchain =
        GFX_CALL(create_swapchain,
            nullptr,
            tex_info,
            GFX_RI_FRAMES_IN_FLIGHT,
            TEXTURE_USAGE_SWAPCHAIN_OWNED | TEXTURE_USAGE_TRANSFER_DST | TEXTURE_USAGE_COLOR,
            format::R8G8B8A8_SRGB,
            PRESENT_MODE_FIFO);

    sm_swapchainWidth = swapchain_width;
    sm_swapchainHeight = swapchain_height;

    // Create our texture views and fences
    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
    {
        sm_swapchainViews[idx] = sm_swapchain.get_image(idx)->create_view(
            format::R8G8B8A8_SRGB,
            RESOURCE_VIEW_2D,
            { 0, 1, 0, 1 });
        sm_frameInFlightFences[idx] = GFX_CALL(create_fence, true);

        std::string frame_dep_name = std::format("FRAME_{}_READY", idx);
        char* frame_dep_cstr = new char[frame_dep_name.size()];
        strcpy(frame_dep_cstr, frame_dep_name.c_str());
        sm_frameInFlightDeps[idx] = GFX_CALL(create_dependency, frame_dep_cstr);
        
        // TODO this is leaked, I don't care. Just put the std::string on the dependency man
        std::string swapchain_dep_name = std::format("SWAPCHAIN_IMAGE_{}_READY", idx);
        char* swapchain_dep_cstr = new char[swapchain_dep_name.size() + 1];
        strcpy(swapchain_dep_cstr, swapchain_dep_name.c_str());
        sm_swapchainImageReady[idx] = GFX_CALL(create_dependency, swapchain_dep_cstr);

        sm_commandLists[idx] = GFX_CALL(allocate_graphics_command_list);
    }
}

void render_interface::shutdown()
{
    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
    {
        texture_view::destroy(&sm_swapchainViews[idx]);
        GFX_CALL(free_dependency, &sm_swapchainImageReady[idx]);
        GFX_CALL(free_dependency, &sm_frameInFlightDeps[idx]);
        GFX_CALL(free_fence, &sm_frameInFlightFences[idx]);
        GFX_CALL(free_command_list, &sm_commandLists[idx]);
    }

    GFX_CALL(free_swapchain, &sm_swapchain);
}

void render_interface::wait_for_frame()
{
    sm_frameInFlightFences[sm_currentFrameIndex].wait();
    sm_frameInFlightFences[sm_currentFrameIndex].reset();
}

void render_interface::begin_frame(bool wait_frame)
{
    GFX_ASSERT(!sm_isFrameActive, "A previous frame is still active, cannot start another.");

    if( wait_frame )
    {
        wait_for_frame();
    }

    handle_swapchain_changes();

    // Bad?
    sm_currentSwapchainReadyDep = &sm_swapchainImageReady[sm_currentFrameIndex];

    swapchain_acquire_result acquire_result =
        GFX_CALL(acquire_next_image, &sm_swapchain, &sm_activeSwapchainImageIndex, sm_currentSwapchainReadyDep, nullptr);

    if( acquire_result != SWAPCHAIN_ACQUIRE_SUCCESS )
    {
        bool swapchain_changed = handle_swapchain_changes(acquire_result == SWAPCHAIN_ACQUIRE_OUT_OF_DATE);

        if( swapchain_changed )
        {
            acquire_result =
                GFX_CALL(acquire_next_image, &sm_swapchain, &sm_activeSwapchainImageIndex, sm_currentSwapchainReadyDep, nullptr);
        }
    }

    GFX_ASSERT(acquire_result == SWAPCHAIN_ACQUIRE_SUCCESS, "Failed to begin frame as next swapchain image could not be acquired.");
    sm_isFrameActive = true;

    sm_commandLists[sm_currentFrameIndex].reset(false);
    sm_commandLists[sm_currentFrameIndex].begin();
}

void render_interface::end_frame()
{
    GFX_ASSERT(sm_isFrameActive, "Frame must be active in order to end it.");

    graphics_command_list* list = &sm_commandLists[sm_currentFrameIndex];

    texture* swapchain_texture = sm_swapchain.get_image(sm_activeSwapchainImageIndex);

    if( swapchain_texture->get_layout() != TEXTURE_LAYOUT_PRESENT )
        list->texture_memory_barrier(swapchain_texture, TEXTURE_LAYOUT_PRESENT);

    list->end();
    list->set_signal_dependency(&sm_frameInFlightDeps[sm_currentFrameIndex]);
    list->add_wait_dependency(sm_currentSwapchainReadyDep);
    list->submit(&sm_frameInFlightFences[sm_currentFrameIndex]);

    sm_swapchain.present(sm_activeSwapchainImageIndex, { &sm_frameInFlightDeps[sm_currentFrameIndex] });

    sm_frameCount++;
    sm_currentFrameIndex = (sm_currentFrameIndex + 1) % GFX_RI_FRAMES_IN_FLIGHT;
    sm_isFrameActive = false;
    sm_currentSwapchainReadyDep = nullptr;

}

void render_interface::recreate_swapchain()
{
    driver::wait_idle();

    texture_info tex_info(sm_swapchainWidth, sm_swapchainHeight, 1, 1);
    swapchain new_swapchain =
        GFX_CALL(create_swapchain,
            &sm_swapchain,
            tex_info,
            GFX_RI_FRAMES_IN_FLIGHT,
            TEXTURE_USAGE_SWAPCHAIN_OWNED | TEXTURE_USAGE_TRANSFER_DST | TEXTURE_USAGE_COLOR,
            format::R8G8B8A8_SRGB,
            PRESENT_MODE_FIFO);

    // Destroy all our old texture views and create the new ones.
    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
    {
        texture_view::destroy(&sm_swapchainViews[idx]);

        sm_swapchainViews[idx] = new_swapchain.get_image(idx)->create_view(
            format::R8G8B8A8_SRGB,
            RESOURCE_VIEW_2D,
            { 0, 1, 0, 1 });
    }

    GFX_CALL(free_swapchain, &sm_swapchain);
    sm_swapchain = std::move(new_swapchain);
}

void render_interface::set_target_swapchain_extents(u32 width, u32 height)
{
    sm_targetSwapchainWidth = width;
    sm_targetSwapchainHeight = height;
}

u32 render_interface::get_current_frame_index()
{
    return sm_currentFrameIndex;
}

u32 render_interface::get_last_frame_index()
{
    return (get_current_frame_index() + GFX_RI_FRAMES_AHEAD) % GFX_RI_FRAMES_IN_FLIGHT;
}

texture_view* render_interface::get_swapchain_texture_view(u32 index)
{
    GFX_ASSERT(index < GFX_RI_FRAMES_IN_FLIGHT, "Index is out of the range of swapchain images.");
    return &sm_swapchainViews[index];
}

texture_view* render_interface::get_active_swapchain_texture_view()
{
    return get_swapchain_texture_view(sm_activeSwapchainImageIndex);
}

dependency* render_interface::get_swapchain_image_ready_dependency(u32 index)
{
    GFX_ASSERT(index < GFX_RI_FRAMES_IN_FLIGHT, "Index is out of the range of swapchain images.");
    return &sm_swapchainImageReady[index];
}

dependency* render_interface::get_current_swapchain_image_ready_dependency()
{
    return get_swapchain_image_ready_dependency(get_current_frame_index());
}

graphics_command_list* render_interface::get_list_temp()
{
    return &sm_commandLists[sm_currentFrameIndex];
}

bool render_interface::handle_swapchain_changes(bool force_recreate)
{
    screen_capabilities capabilities = driver::query_screen_capabilities();
    if( capabilities.current_width == u32_max )
        return false;

    if( capabilities.current_width != sm_swapchainWidth
        || capabilities.current_height != sm_swapchainHeight
        || force_recreate )
    {
        sm_swapchainWidth = capabilities.current_width;
        sm_swapchainHeight = capabilities.current_height;
        recreate_swapchain();
        return true;
    }

    return false;
}

// Static initialization
swapchain render_interface::sm_swapchain{ };

u32 render_interface::sm_targetSwapchainWidth = 0;
u32 render_interface::sm_targetSwapchainHeight = 0;
u32 render_interface::sm_swapchainWidth = 0;
u32 render_interface::sm_swapchainHeight = 0;

bool render_interface::sm_isFrameActive = false;
u32 render_interface::sm_currentFrameIndex = 0;
u32 render_interface::sm_frameCount = 0;
u32 render_interface::sm_activeSwapchainImageIndex = 0;

texture_view render_interface::sm_swapchainViews[GFX_RI_FRAMES_IN_FLIGHT] = { };

dependency render_interface::sm_swapchainImageReady[GFX_RI_FRAMES_IN_FLIGHT] = { };
dependency* render_interface::sm_currentSwapchainReadyDep = nullptr;

dependency render_interface::sm_frameInFlightDeps[GFX_RI_FRAMES_IN_FLIGHT] = { };
fence render_interface::sm_frameInFlightFences[GFX_RI_FRAMES_IN_FLIGHT] = { };

graphics_command_list render_interface::sm_commandLists[GFX_RI_FRAMES_IN_FLIGHT] = { };

} // fw
} // gfx