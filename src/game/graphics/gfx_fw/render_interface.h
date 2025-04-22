#pragma once
#include "../gfx_core/swapchain.h"
#include "../gfx_core/fence.h"
#include "../gfx_core/dependency.h"
#include "../gfx_core/resource_view.h"
#include "../gfx_core/command_list.h"

#include "context.h"

namespace gfx
{

namespace fw
{

#define GFX_RI_FRAMES_AHEAD 2
#define GFX_RI_FRAMES_IN_FLIGHT GFX_RI_FRAMES_AHEAD + 1

#define GFX_RI_RENDER_THREADS 1

#define RI_GraphicsContext ::gfx::fw::render_interface::get_graphics_context()

class render_interface
{
public:
    static void initialise();
    static void shutdown();

    static void reset_device();
    static void recreate_swapchain();

    static void wait_for_frame();
    static void begin_frame(bool wait_frame = true);
    static void end_frame();

    static void set_target_swapchain_extents(u32 width, u32 height);

    static u32 get_current_frame_index();
    static u32 get_last_frame_index();

    // Do the names really have to be this bad?
    static texture_view* get_swapchain_texture_view(u32 index);
    static texture_view* get_active_swapchain_texture_view();

    static dependency* get_swapchain_image_ready_dependency(u32 index);
    static dependency* get_current_swapchain_image_ready_dependency();

    static void force_recreate_swapchain();

    static graphics_context& get_graphics_context();
private:
    static bool handle_swapchain_changes(bool force_recreate = false);
private:
    static swapchain sm_swapchain;
    static u32 sm_targetSwapchainWidth, sm_targetSwapchainHeight;
    static u32 sm_swapchainWidth, sm_swapchainHeight;
    static bool sm_forceRecreateSwapchain;

    static bool sm_isFrameActive;
    static u32 sm_currentFrameIndex;
    static u32 sm_frameCount;
    static u32 sm_activeSwapchainImageIndex;

    static texture_view sm_swapchainViews[GFX_RI_FRAMES_IN_FLIGHT];

    static dependency sm_swapchainImageReady[GFX_RI_FRAMES_IN_FLIGHT];
    static dependency* sm_currentSwapchainReadyDep;

    static dependency sm_frameInFlightDeps[GFX_RI_FRAMES_IN_FLIGHT];
    static fence sm_frameInFlightFences[GFX_RI_FRAMES_IN_FLIGHT];

    static thread_local graphics_context sm_graphicsContext;
    static std::vector<graphics_command_list*> sm_graphicsListsToSubmit;
    static graphics_command_list sm_graphicsSubmissionLists[GFX_RI_FRAMES_IN_FLIGHT];

    static graphics_command_list sm_graphicsContextCommandLists[GFX_RI_FRAMES_IN_FLIGHT][GFX_RI_RENDER_THREADS];
private:
    // Context management functions
    static void begin_context();
    static void end_context();
};

} // fw
} // gfx