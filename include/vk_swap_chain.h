//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VK_SWAP_CHAIN_H
#define PHY_SIM_VK_SWAP_CHAIN_H

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

class VulkanSwapChain{
public:
    auto getImageViews() const{
        return m_imageViews;
    }

    auto getFormat() const{
        return m_imageFormat;
    }

    auto getExtent() const {
        return m_extent;
    }

    auto getSwapChain() const{
        return m_swapChain;
    }

    auto getImagesNum() const{
        return m_images.size();
    }

    /*
    auto getFrameBufferAt(uint32_t index) const{
        return m_framebuffers[index];
    }

    auto getShadowFrameBufferAt(uint32_t index) const {
        return m_shadow_framebuffers[index];
    }

    auto getGUIFrameBufferAt(uint32_t index) const{
        return m_gui_framebuffers[index];
    }

    auto getFrameBuffersNum() const{
        return m_framebuffers.size();
    }
     */

    void createSwapChain(VkSwapchainKHR oldSwapchain,VkPhysicalDevice physicalDevice,VkSurfaceKHR surface,GLFWwindow* window) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities,window);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0) {
            imageCount = std::min(swapChainSupport.capabilities.maxImageCount, imageCount);
        }

        VkSwapchainCreateInfoKHR createInfo = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = surface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };

        //specify how to handle swap chain images that will be used across multiple queue families
        auto indices = VulkanDeviceManager::instance().m_physicalDevices[0].findQueueFamilies(surface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapchain;//would be used if window resize

        if (vkCreateSwapchainKHR(VulkanDeviceManager::instance().getDevice(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swapChain!");
        }
        vkGetSwapchainImagesKHR(VulkanDeviceManager::instance().getDevice(), m_swapChain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(VulkanDeviceManager::instance().getDevice(), m_swapChain, &imageCount, m_images.data());
        m_imageFormat = surfaceFormat.format;
        m_extent = extent;
    }

    void createImageViews() {
        m_imageViews.resize(m_images.size());
        for (size_t i = 0; i < m_images.size(); i++) {
            m_imageViews[i] = createImageView(m_images[i], m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT,1);
        }
    }

    /*
    void createFramebuffers(VkImageView color,VkImageView depth,VkRenderPass renderPass) {

        auto swapChainImageViewsSize = m_imageViews.size();
        m_framebuffers.resize(swapChainImageViewsSize);

        for (size_t i = 0; i < swapChainImageViewsSize; i++) {
            VkImageView attachments[] = {color, depth,m_imageViews[i]};
            VkFramebufferCreateInfo framebufferInfo = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = static_cast<uint32_t>(std::size(attachments)),
                    .pAttachments = attachments,
                    .width = m_extent.width,
                    .height = m_extent.height,
                    .layers = 1
            };

            if (vkCreateFramebuffer(VulkanDeviceManager::instance().getDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createGUIFramebuffers(VkRenderPass renderPass) {
        auto swapChainImageViewsSize = m_imageViews.size();
        m_gui_framebuffers.resize(swapChainImageViewsSize);

        for(size_t i = 0; i < swapChainImageViewsSize; i++){
            VkImageView attachments[1] = {m_imageViews[i]};
            VkFramebufferCreateInfo framebufferInfo = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = m_extent.width,
                    .height = m_extent.height,
                    .layers = 1
            };

            if (vkCreateFramebuffer(VulkanDeviceManager::instance().getDevice(), &framebufferInfo, nullptr, &m_gui_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create gui framebuffer!");
            }
        }
    }

    void createShadowFrambuffers(VkImageView shadowMapView,VkRenderPass renderPass){
        auto swapChainImageViewsSize = m_imageViews.size();
        m_shadow_framebuffers.resize(swapChainImageViewsSize);
        for(size_t i = 0; i < swapChainImageViewsSize; i++){
            VkImageView attachments[1] = {shadowMapView};
            VkFramebufferCreateInfo framebufferInfo = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = renderPass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = m_extent.width*2,
                    .height = m_extent.height*2,
                    .layers = 1
            };
            if (vkCreateFramebuffer(VulkanDeviceManager::instance().getDevice(), &framebufferInfo, nullptr, &m_shadow_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shadow framebuffer!");
            }
        }
    }

    void destroyAllFrameBuffers(){
        for (auto framebuffer: m_framebuffers) {
            VulkanDeviceManager::destroyObject(vkDestroyFramebuffer,framebuffer);
        }
        for(auto framebuffer : m_gui_framebuffers){
            VulkanDeviceManager::destroyObject(vkDestroyFramebuffer,framebuffer);
        }
    }
    */

    void destoryImageViews(){
        for (auto imageView: m_imageViews) {
            VulkanDeviceManager::destroyObject(vkDestroyImageView,imageView);
        }
    }

    void destory(){
        VulkanDeviceManager::destroyObject(vkDestroySwapchainKHR,m_swapChain);
    }

private:

    VkSwapchainKHR m_swapChain;

    VkFormat m_imageFormat;
    VkExtent2D m_extent;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;

    //std::vector<VkFramebuffer> m_framebuffers;

    //std::vector<VkFramebuffer> m_gui_framebuffers;

    //std::vector<VkFramebuffer> m_shadow_framebuffers;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avaliableFormats) {
        for (const auto &avaliableFormat: avaliableFormats) {
            if (avaliableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                avaliableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                return avaliableFormat;
            }
        }
        return avaliableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &avaliableModes) {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto &avaliableMode: avaliableModes) {
            if (avaliableMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return avaliableMode;
            } else if (avaliableMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = avaliableMode;
            }
        }
        return bestMode;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

            actualExtent.width = std::max(capabilities.minImageExtent.width,
                                          std::min(actualExtent.width, capabilities.maxImageExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height,
                                           std::min(actualExtent.height, capabilities.maxImageExtent.height));

            return actualExtent;
        }
    }
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectflags, uint32_t mipLevels) const {
        VkImageViewCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = format,
                .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
                .subresourceRange.aspectMask = aspectflags,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = mipLevels,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1
        };


        VkImageView imageview;
        if (vkCreateImageView(VulkanDeviceManager::instance().getDevice(), &createInfo, nullptr, &imageview) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        return imageview;
    }
};
#endif //PHY_SIM_VK_SWAP_CHAIN_H
