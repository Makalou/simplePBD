//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VK_COMMAND_MANAGER_H
#define PHY_SIM_VK_COMMAND_MANAGER_H

#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"
#include <future>
#include <optional>

class VulkanCommandPool{
public:
    VulkanCommandPool() = default;
    std::vector<VkCommandBuffer> allocateCommandBuffers(VkCommandBufferLevel level, uint32_t count) const{
        std::vector<VkCommandBuffer> commandBuffers(count);
        VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = m_pool,
                .level = level,
                .commandBufferCount = count
        };

        if (vkAllocateCommandBuffers(m_device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        return commandBuffers;
    }

    VkCommandBuffer allocateCommandBuffer(VkCommandBufferLevel level) const{
        VkCommandBuffer commandBuffer;
        VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = m_pool,
                .level = level,
                .commandBufferCount = 1
        };

        if (vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        return commandBuffer;
    }

    void freeBuffers(VkCommandBuffer* pCommandBuffers,uint32_t commandBufferCount){
        vkFreeCommandBuffers(m_device,m_pool, commandBufferCount, pCommandBuffers);
    }

    void freeBuffers(const std::vector<VkCommandBuffer> & commandBuffers) const{
        vkFreeCommandBuffers(m_device,m_pool, commandBuffers.size(), commandBuffers.data());
    }

    void destroy(){
        vkDestroyCommandPool(m_device,m_pool, nullptr);
    }

private:
    friend class CommandPoolBuilder;
    VulkanCommandPool(VkDevice device, VkCommandPool pool){
        m_device = device;
        m_pool = pool;
    }
    VkCommandPool m_pool;
    VkDevice m_device;
};

class CommandPoolBuilder{
public:
    explicit CommandPoolBuilder(vkb::Device device){
        m_device = device;
        queueType = vkb::QueueType::graphics;
        m_flags = 0;
    }

    CommandPoolBuilder& setQueue(vkb::QueueType type){
        queueType = type;
        return *this;
    }

    CommandPoolBuilder& setFlags(VkCommandPoolCreateFlags flags){
        m_flags = flags;
        return *this;
    }

    std::optional<VulkanCommandPool> build() const{
        uint32_t queueFamilyIdx = m_device.get_queue_index(queueType).value();
        VkCommandPoolCreateInfo poolInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = m_flags,
                .queueFamilyIndex = queueFamilyIdx,
        };

        VkCommandPool pool;

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
            return {};
            throw std::runtime_error("failed to create command pool!");
        }

        return VulkanCommandPool{m_device,pool};
    }
private:
    vkb::Device m_device;
    vkb::QueueType queueType;
    VkCommandPoolCreateFlags m_flags;
};

class VulkanSingleCommandPool{
public:
    VkCommandBuffer beginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
        };

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void endandSubmitSingleTimeCommands(VkCommandBuffer commandBuffer) const {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };

        auto graphic_queue = m_device.get_queue(vkb::QueueType::graphics).value();
        vkQueueSubmit(graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);

        std::async([&]{
            vkQueueWaitIdle(graphic_queue);
            vkFreeCommandBuffers(m_device, pool, 1, &commandBuffer);
        });
    }

private:
    static VkCommandPool pool;
    static vkb::Device m_device;
};

class VulkanCommandManager
{
public:
    void setDevice(vkb::Device device){
        m_device = device;
        VkCommandPoolCreateInfo poolInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = 0,
                .queueFamilyIndex = m_device.get_queue_index(vkb::QueueType::graphics).value(),
        };

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    /*
    void createCommandPools(uint32_t queueFamilyIdx) {
        VkCommandPoolCreateInfo poolInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = 0,
                .queueFamilyIndex = queueFamilyIdx,
        };

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void createGUICommandPools(uint32_t count,uint32_t queueFamilyIdx){
        gui_commandPools.resize(count);
        for(auto i = 0;i<count;++i){
            VkCommandPoolCreateInfo poolInfo = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                    .queueFamilyIndex = queueFamilyIdx
            };
            if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &gui_commandPools[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create gui command pool!"+std::to_string(i));
            }
        }
    }

    void createCommandBuffers(uint32_t count){
        commandBuffers.resize(count);//todo why?
        VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = (uint32_t) commandBuffers.size()
        };

        if (vkAllocateCommandBuffers(m_device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void createGUICommandBuffers(uint32_t count){
        gui_commandBuffers.resize(count);
        for(auto i = 0;i<count;++i){
            VkCommandBufferAllocateInfo allocInfo = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = gui_commandPools[i],
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1
            };
            if (vkAllocateCommandBuffers(m_device, &allocInfo, &gui_commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate command buffers!");
            }
        }
    }

    void freeBuffers(){
        vkFreeCommandBuffers(m_device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    }

    void freeGuiCommandBuffers(){
        for(auto i =0;i<gui_commandBuffers.size();++i){
            vkFreeCommandBuffers(m_device, gui_commandPools[i], 1, &gui_commandBuffers[i]);
        }

    }

    void destroyAllCommandPools(){
        vkDestroyCommandPool(m_device,commandPool, nullptr);
        for(auto i =0;i<gui_commandPools.size();++i) {
            vkDestroyCommandPool(m_device,gui_commandPools[i], nullptr);
        }
    }

    auto getCommandBufferAt(size_t idx){
        return commandBuffers[idx];
    }

    auto getGUICommandBufferAt(size_t idx){
        return gui_commandBuffers[idx];
    }

    auto getCommandBufferCount() const {
        return commandBuffers.size();
    }
     */

    VkCommandBuffer beginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
        };

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void endandSubmitSingleTimeCommands(VkCommandBuffer commandBuffer) const {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };

        auto gq = m_device.get_queue(vkb::QueueType::graphics).value();
        vkQueueSubmit(gq, 1, &submitInfo, VK_NULL_HANDLE);

        std::async([&]{
            vkQueueWaitIdle(gq);
            vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
        });
    }

    static VulkanCommandManager& instance(){
        static VulkanCommandManager _instance;
        return _instance;
    }

private:
    vkb::Device m_device;
    VulkanCommandManager()=default;
    VkCommandPool commandPool{};
    std::vector<VkCommandPool>  gui_commandPools{};
    std::vector<VkCommandBuffer> commandBuffers{};
    std::vector<VkCommandBuffer> gui_commandBuffers{};
};

#endif //PHY_SIM_VK_COMMAND_MANAGER_H
