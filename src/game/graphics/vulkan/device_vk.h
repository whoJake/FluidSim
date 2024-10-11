#pragma once

#include "gfx_core/device.h"
#include "gfx_core/gpu.h"

namespace gfx
{

class device_vk : public device
{
public:
    device_vk();
    ~device_vk() override;

    u32 initialise(u32 gpuIdx) override;
    void shutdown() override;

    std::vector<gpu> enumerate_gpus() const override;

    void wait_idle() override;
private:
    void create_instance();

    std::vector<const char*> get_instance_extensions() const;
    std::vector<const char*> get_instance_layers() const;
    std::vector<const char*> get_device_extensions() const;
private:
    VkInstance m_instance;
    std::vector<const char*> m_enabledInstanceExtensions;
    std::vector<const char*> m_enabledInstanceLayers;
    
    gpu m_gpu;
    VkDevice m_device;
};

} // gfx