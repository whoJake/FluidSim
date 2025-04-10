#pragma once
#include "../gfx_core/swapchain.h"
#include "../gfx_core/fence.h"
#include "../gfx_core/dependency.h"
#include "../gfx_core/resource_view.h"
#include "../gfx_core/command_list.h"

namespace gfx
{

namespace fw
{

#define GFX_RI_FRAMES_AHEAD 1
#define GFX_RI_FRAMES_IN_FLIGHT GFX_RI_FRAMES_AHEAD + 1

class render_interface
{
public:
    static void initialise();
    static void shutdown();

    static void reset_device();
    static void recreate_swapchain();

    static void begin_frame();
    static void end_frame();

    static void set_target_swapchain_extents(u32 width, u32 height);

    static u32 get_current_frame_index();
    static u32 get_last_frame_index();

    // Do the names really have to be this bad?
    static texture_view* get_swapchain_texture_view(u32 index);
    static texture_view* get_active_swapchain_texture_view();

    static dependency* get_swapchain_image_ready_dependency(u32 index);
    static dependency* get_current_swapchain_image_ready_dependency();

    static graphics_command_list* get_list_temp();
private:
    static bool handle_swapchain_changes(bool force_recreate = false);
private:
    static swapchain sm_swapchain;
    static u32 sm_targetSwapchainWidth, sm_targetSwapchainHeight;
    static u32 sm_swapchainWidth, sm_swapchainHeight;

    static bool sm_isFrameActive;
    static u32 sm_currentFrameIndex;
    static u32 sm_frameCount;
    static u32 sm_activeSwapchainImageIndex;

    static texture_view sm_swapchainViews[GFX_RI_FRAMES_IN_FLIGHT];

    static dependency sm_swapchainImageReady[GFX_RI_FRAMES_IN_FLIGHT];
    static dependency* sm_currentSwapchainReadyDep;

    static dependency sm_frameInFlightDeps[GFX_RI_FRAMES_IN_FLIGHT];
    static fence sm_frameInFlightFences[GFX_RI_FRAMES_IN_FLIGHT];

    // TEMP
    static graphics_command_list sm_commandLists[GFX_RI_FRAMES_IN_FLIGHT];
};

} // fw
} // gfx