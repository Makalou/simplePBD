//
// Created by 王泽远 on 2022/7/1.
//

#ifndef PHY_SIM_VK_DEVICE_H
#define PHY_SIM_VK_DEVICE_H

#include "vk_command_manager.h"

class VulkanDevice: public vkb::Device{
public:

    explicit VulkanDevice(vkb::Device device){
        m_device = device;
    }

    void build_command_pool(vkb::QueueType type) {
        CommandPoolBuilder builder{m_device};
        auto pool = builder.setQueue(type).build().value();
        switch (type) {
            case vkb::QueueType::present:
                m_present_command_pools.push_back(pool);
                break;
            case vkb::QueueType::graphics:
                m_graphics_command_pools.push_back(pool);
                break;
            case vkb::QueueType::compute:
                m_compute_command_pools.push_back(pool);
                break;
            case vkb::QueueType::transfer:
                m_transfer_command_pools.push_back(pool);
                break;
            default:
                break;
        }
    }

    std::vector<VulkanCommandPool> get_command_pool_by_type(vkb::QueueType type) const{
        switch (type) {
            case vkb::QueueType::present:
                return m_present_command_pools;
            case vkb::QueueType::graphics:
                return m_graphics_command_pools;
            case vkb::QueueType::compute:
                return m_compute_command_pools;
            case vkb::QueueType::transfer:
                return m_transfer_command_pools;
            default:
                return {};
        }
    }


private:
    vkb::Device m_device;

    std::vector<VulkanCommandPool> m_present_command_pools;
    std::vector<VulkanCommandPool> m_graphics_command_pools;
    std::vector<VulkanCommandPool> m_compute_command_pools;
    std::vector<VulkanCommandPool> m_transfer_command_pools;
};
#endif //PHY_SIM_VK_DEVICE_H
