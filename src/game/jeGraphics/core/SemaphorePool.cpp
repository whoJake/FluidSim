#include "SemaphorePool.h"
#include "Device.h"

namespace vk
{

SemaphorePool::SemaphorePool(Device& device) :
    m_device(device)
{ }

SemaphorePool::~SemaphorePool()
{
    reset();

    for( VkSemaphore semaphore : m_sempahores )
    {
        vkDestroySemaphore(m_device.get_handle(), semaphore, nullptr);
    }

    m_sempahores.clear();
}

void SemaphorePool::reset()
{
    m_activeSemaphores = 0;

    for( VkSemaphore semaphore : m_releasedSemaphores )
    {
        m_sempahores.push_back(semaphore);
    }

    m_releasedSemaphores.clear();
}

VkSemaphore SemaphorePool::request_semaphore()
{
    if( m_activeSemaphores < m_sempahores.size() )
    {
        return m_sempahores[m_activeSemaphores++];
    }

    VkSemaphore createdSemaphore{ VK_NULL_HANDLE };
    VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    createInfo.flags = 0;

    VkResult createResult = vkCreateSemaphore(m_device.get_handle(), &createInfo, nullptr, &createdSemaphore);
    VK_CHECK(createResult, "Failed to create new semaphore in SemaphorePool.");

    m_sempahores.push_back(createdSemaphore);
    m_activeSemaphores++;
    return createdSemaphore;
}

VkSemaphore SemaphorePool::request_semaphore_with_ownership()
{
    if( m_activeSemaphores < m_sempahores.size() )
    {
        VkSemaphore semaphore = m_sempahores.back();
        m_sempahores.pop_back();
        return semaphore;
    }

    VkSemaphore createdSemaphore{ VK_NULL_HANDLE };
    VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    createInfo.flags = 0;

    VkResult createResult = vkCreateSemaphore(m_device.get_handle(), &createInfo, nullptr, &createdSemaphore);
    VK_CHECK(createResult, "Failed to create new owned semaphore in SemaphorePool.");

    return createdSemaphore;
}

void SemaphorePool::release_owned_semaphore(VkSemaphore semaphore)
{
    m_releasedSemaphores.push_back(semaphore);
}

uint32_t SemaphorePool::get_active_semaphore_count() const
{
    return m_activeSemaphores;
}

} // vk