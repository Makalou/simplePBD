#pragma once

#include "vulkan/vulkan.hpp"
#include "vk_resource_manager.h"
#include "vk_graphics_pipeline.h"
#include "vk_command_manager.h"
#include "GLFW/glfw3.h"
#include <vector>

#include "VkBootstrap.h"

const size_t MAX_FRAME_IN_FLIGHT = 3;

class VulkanContext {
public:
    void init(GLFWwindow *window);

    void cleanup();

    virtual void recreateSwapChain() = 0;

protected:
    virtual void init_resource() = 0;
    virtual void clean_resource() = 0;
public:

    VkSwapchainKHR get_swapChain() const {
        return vkb_swapchain;
    }

    auto getDevice() const{
        return vkb_device;
    }

    void createSyncObjects();

    template<typename Func, typename ... Args>
    void destroyDeviceObject(Func f, Args... args){
        f(vkb_device,args..., nullptr);
    }

public:
    GLFWwindow *window;

    vkb::Instance vkb_inst;
    vkb::Device vkb_device;
    VkSurfaceKHR surface;
    vkb::Swapchain vkb_swapchain;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
};

class MyContext: public VulkanContext{
    void init_resource() override;

    void clean_resource() override;

    void recreateSwapChain() override;

    void createFramebuffers(VkImageView color,VkImageView depth,VkRenderPass renderPass) {

        auto swapChainImageViewsSize = vkb_swapchain.image_count;
        m_framebuffers.resize(swapChainImageViewsSize);

        auto image_views = vkb_swapchain.get_image_views().value();

        for (size_t i = 0; i < swapChainImageViewsSize; i++) {
            VkImageView attachments[] = {color, depth,image_views[i]};
            VkFramebufferCreateInfo framebufferInfo = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = static_cast<uint32_t>(std::size(attachments)),
                    .pAttachments = attachments,
                    .width = vkb_swapchain.extent.width,
                    .height = vkb_swapchain.extent.height,
                    .layers = 1
            };

            if (vkCreateFramebuffer(vkb_device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createGUIFramebuffers(VkRenderPass renderPass) {
        auto swapChainImageViewsSize = vkb_swapchain.image_count;
        m_gui_framebuffers.resize(swapChainImageViewsSize);

        for(size_t i = 0; i < swapChainImageViewsSize; i++){
            VkImageView attachments[1] = {vkb_swapchain.get_image_views().value()[i]};
            VkFramebufferCreateInfo framebufferInfo = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = vkb_swapchain.extent.width,
                    .height = vkb_swapchain.extent.height,
                    .layers = 1
            };

            if (vkCreateFramebuffer(vkb_device, &framebufferInfo, nullptr, &m_gui_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create gui framebuffer!");
            }
        }
    }

    void createShadowFrambuffers(VkImageView shadowMapView,VkRenderPass renderPass){
        auto swapChainImageViewsSize = vkb_swapchain.image_count;
        m_shadow_framebuffers.resize(swapChainImageViewsSize);
        for(size_t i = 0; i < swapChainImageViewsSize; i++){
            VkImageView attachments[1] = {shadowMapView};
            VkFramebufferCreateInfo framebufferInfo = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = vkb_swapchain.extent.width * 2,
                    .height = vkb_swapchain.extent.height * 2,
                    .layers = 1
            };
            if (vkCreateFramebuffer(vkb_device, &framebufferInfo, nullptr, &m_shadow_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shadow framebuffer!");
            }
        }
    }

    void createRenderPass();

    void createImGuiRenderPass();

    void createShadowPass();

    void createTextureSampler();

    void createDescriptorPool(uint32_t count);

    void createDescriptorSets(uint32_t count);

    void recordCommandBuffers();

    void recordShadowPass(VkCommandBuffer commandBuffer,uint32_t des_idx);

    void recordForwardPass(VkCommandBuffer commandBuffer,uint32_t des_idx);

    void cleanupSwapChain();

    void destroyAllFrameBuffers(){
        for (auto framebuffer: m_framebuffers) {
            destroyDeviceObject(vkDestroyFramebuffer,framebuffer);
        }
        for(auto framebuffer : m_gui_framebuffers){
            destroyDeviceObject(vkDestroyFramebuffer,framebuffer);
        }
    }

public:
    VulkanResourceManager resourceManager;

    VulkanPipeline graphicsPipeline;
    VulkanPipeline shadowPipeline;

    VulkanCommandPool commandPool;
    std::vector<VulkanCommandPool>  gui_commandPools{};

    std::vector<VkCommandBuffer> commandBuffers{};
    std::vector<VkCommandBuffer> gui_commandBuffers{};

    std::vector<VkFramebuffer> m_framebuffers;
    std::vector<VkFramebuffer> m_gui_framebuffers;
    std::vector<VkFramebuffer> m_shadow_framebuffers;

    VkRenderPass renderPass;
    VkRenderPass imGuiRenderPass;
    VkRenderPass shadowPass;

    VkDescriptorSetLayout descriptorSetLayout;

    VkSampler textureSampler;
    VkSampler shadowMapSampler;
    uint32_t mipLevels;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDescriptorSet> descriptorSets;
    VkDescriptorPool descriptorPool;
    VkDescriptorPool imguiDescriptorPool;
};
