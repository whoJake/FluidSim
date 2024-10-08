#pragma once

#include "entity.h"
#include "data/pool.h"

namespace fw
{

/// <summary>
/// Globally initialised container for entities.
/// </summary>
class world
{
public:
protected:
    world();
    ~world();

private:
    std::vector<mtl::pool<entity>> m_entities;
public:
    static u32 initialise();
    static void shutdown();
private:
    static world* m_instance;
};

} // fw