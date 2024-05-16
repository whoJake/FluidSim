#pragma once

namespace vk
{

class Device;

template<typename THandle>
class Resource
{
public:
    Resource(THandle handle, Device& device) :
        m_handle(handle),
        m_device(device)
    { }
    virtual ~Resource() = default;

    Resource(const Resource& other) = delete;
    Resource(Resource&& other) :
        m_handle(other.m_handle),
        m_device(other.m_device)
    {
        other.m_handle = VK_NULL_HANDLE;
    }

    Resource& operator=(Resource&& other) = delete;
    Resource& operator=(const Resource& other) = delete;

    inline const THandle& get_handle() const
    {
        return m_handle;
    }

    inline Device& get_device() const
    {
        return m_device;
    }
protected:
    THandle m_handle;
    Device& m_device;
};

} // vk