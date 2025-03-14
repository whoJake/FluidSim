#pragma once
#include "system/memory_zone.h"

// This shouldn't need to be here..
#include "system/zone_allocator.h"

enum gfx_memory_zone : u32
{
    MEMZONE_GFX_BEGIN = MEMZONE_SYSTEM_END,

    MEMZONE_GFX_DEFAULT,
    MEMZONE_GFX_SHADERS,

    MEMZONE_GFX_END = MEMZONE_GFX_SHADERS,
    MEMZONE_GFX_COUNT = MEMZONE_GFX_END - MEMZONE_GFX_BEGIN,
};

void initialise_gfx_zones()
{
    sys::zone_allocator::register_zone("graphics.default", MEMZONE_GFX_DEFAULT, { 0, sys::budget_failure_type::silent });
    sys::zone_allocator::register_zone("graphics.shaders", MEMZONE_GFX_SHADERS, { 0, sys::budget_failure_type::silent });
}