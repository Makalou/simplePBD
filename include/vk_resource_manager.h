//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VK_RESOURCE_MANAGER_H
#define PHY_SIM_VK_RESOURCE_MANAGER_H

#include "vulkan/vulkan.hpp"
#include "vertex.h"
#include "UniformBufferObject.h"
#include "vulkan_memory_utils.h"
#include "vk_command_manager.h"

struct vertexPositionView{
public:
    void operator=(const glm::vec3 & p){
        for(auto & p_pos : p_positions)
            *p_pos = p;
    }
    glm::vec3& operator*(){
        return *p_positions[0];
    }
    void reg(glm::vec3* p){
        p_positions.push_back(p);
    }

    bool operator == (const glm::vec3 v){
        return v == *p_positions[0];
    }

    void operator +=(const glm::vec3& v){
        for(auto & p_pos : p_positions)
            *p_pos += v;
    }

    void operator -=(const glm::vec3& v){
        for(auto & p_pos : p_positions)
            *p_pos -= v;
    }

    const glm::vec3 * operator->() const {
        return p_positions[0];
    }

private:
    std::vector<glm::vec3*> p_positions;
};

class VulkanResourceManager {
public:
    void setDevice(vkb::Device device){
        m_device = device;
    }
    void createVertexBuffer();

    void updateVertexBuffer();

    void updateVertexBuffer2();

    void createIndexBuffer();

    void createUniformBuffers(size_t ub_size);

    void updateUniformBuffer(uint32_t currentImage, float aspect,bool rotate);

    void createTextureImage(uint32_t &mipLevels);

    void createTextureImageView(uint32_t mipLevels);

    void createColorResources(VkFormat swapChainImageFormat, uint32_t width, uint32_t height,
                              VkSampleCountFlagBits msaaSamples);

    void createDepthResources(uint32_t width, uint32_t height,VkSampleCountFlagBits msaaSamples);

    void createShadowMapResource(uint32_t width,uint32_t height);

    void cleanUpColorResources() {
        vkDestroyImageView(m_device, colorImageView, nullptr);
        vkDestroyImage(m_device, colorImage, nullptr);
        vkFreeMemory(m_device, colorImageMemory, nullptr);
    }

    void cleanUpDepthResources() {
        vkDestroyImageView(m_device, depthImageView, nullptr);
        vkDestroyImage(m_device, depthImage, nullptr);
        vkFreeMemory(m_device, depthImageMemory, nullptr);
    }

    void cleanUpUniformBuffers() {
        for (auto uniformBuffer: uniformBuffers) {
            vkDestroyBuffer(m_device, uniformBuffer, nullptr);
        }

        for (auto memory: uniformBuffersMemory) {
            vkFreeMemory(m_device, memory, nullptr);
        }
    }

    void cleanUp() {
        vkDestroyBuffer(m_device, vertexBuffer, nullptr);
        vkFreeMemory(m_device, vertexBufferMemory, nullptr);
        vkDestroyBuffer(m_device, indexBuffer, nullptr);
        vkFreeMemory(m_device, indexBufferMemory, nullptr);
        vkDestroyBuffer(m_device, quadVertexBuffer, nullptr);
        vkFreeMemory(m_device, quadVertexBufferMemory, nullptr);
        vkDestroyImageView(m_device, textureImageView, nullptr);
        vkDestroyImage(m_device, textureImage, nullptr);
        vkFreeMemory(m_device, textureImageMemory, nullptr);
    }

    auto &getVertices() {
        return vertices;
    }

    auto &getIndies() {
        return indies;
    }

    auto &getUniformBuffers() {
        return uniformBuffers;
    }

    auto getVertexBuffer() const {
        return vertexBuffer;
    }

    auto getVertexBuffer2() const {
        return vertexBuffer2;
    }

    auto getQuadVertexBuffer() const{
        return quadVertexBuffer;
    }

    auto getIndexBuffer() const {
        return indexBuffer;
    }

    auto getTextureImageView() const {
        return textureImageView;
    }

    auto getColorImageView() const {
        return colorImageView;
    }

    auto getDepthImageView() const {
        return depthImageView;
    }

    auto getShadowMapView() const {
        return shadowMapImageView;
    }

    auto getVertexPositionView(){
        return position_views;
    }
    auto getVertexPositionView2(){
        return position_views2;
    }

private:

    void
    createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo bufferInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = size,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex bufer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);
        VkMemoryAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,//the size of the allocation in bytes
                .memoryTypeIndex = findMemoryType(m_device.physical_device, memRequirements.memoryTypeBits, properties)
        };


        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(m_device, buffer, bufferMemory, 0);

    }

    void
    createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
                VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                VkImage &image, VkDeviceMemory &imageMemory) {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = numSamples;
        imageInfo.flags = 0;//for sparse image

        if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, image, &memRequirements);
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(m_device.physical_device, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_device, image, imageMemory, 0);
    }

    VkImageView
    createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectflags, uint32_t mipLevels) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectflags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageview;
        if (vkCreateImageView(m_device, &createInfo, nullptr, &imageview) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        return imageview;
    }

    static void
    copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandbuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandbuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandbuffer);
    }

    static void
    copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandBuffer);
    }

    static bool
    hasStencilComponenet(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    static void
    transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels) {
        VkCommandBuffer commandBuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage;
        VkPipelineStageFlags dstStage;

        if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
            barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) &&(newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) &&(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponenet(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && newLayout == (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)) {
            barrier.srcAccessMask = VK_ACCESS_NONE_KHR;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else {
            throw std::runtime_error("unsupported layout transition!");
        }
        vkCmdPipelineBarrier(commandBuffer,
                             srcStage, dstStage, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandBuffer);
    }

    static void
    generateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
        VkCommandBuffer commandBuffer = VulkanCommandManager::instance().beginSingleTimeCommands();
        VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .image = image,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1,
                .subresourceRange.levelCount = 1,
        };


        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;
        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            VkImageBlit blit = {
                    .srcOffsets[0] = {0, 0, 0},
                    .srcOffsets[1] = {mipWidth, mipHeight, 1},
                    .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .srcSubresource.mipLevel = i - 1,
                    .srcSubresource.baseArrayLayer = 0,
                    .srcSubresource.layerCount = 1,

                    .dstOffsets[0] = {0, 0, 0},
                    .dstOffsets[1] = {mipWidth > 1 ? (mipWidth / 2) : 1, mipHeight > 1 ? (mipHeight / 2) : 1, 1},
                    .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .dstSubresource.mipLevel = i,
                    .dstSubresource.baseArrayLayer = 0,
                    .dstSubresource.layerCount = 1
            };

            vkCmdBlitImage(commandBuffer,
                           image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit, VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier
            );
            if (mipWidth > 1)mipWidth /= 2;
            if (mipHeight > 1)mipHeight /= 2;

        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier
        );

        VulkanCommandManager::instance().endandSubmitSingleTimeCommands(commandBuffer);
    }

    VkFormat
    findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (const auto format: candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_device.physical_device, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat
    findDepthFormat() {
        return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    vkb::Device m_device;
    //device memories
    VkDeviceMemory vertexBufferMemory;
    VkDeviceMemory indexBufferMemory;
    VkDeviceMemory quadVertexBufferMemory;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDeviceMemory textureImageMemory;
    VkDeviceMemory colorImageMemory;
    VkDeviceMemory depthImageMemory;
    VkDeviceMemory shadowMapMemory;

    //std::vector<VkDeviceMemory> m_deviceMemories;

    //buffers
    VkBuffer vertexBuffer;
    VkBuffer vertexBuffer2;
    VkBuffer indexBuffer;
    VkBuffer quadVertexBuffer;
    std::vector<VkBuffer> uniformBuffers;

    //std::unordered_map<std::string ,VkBuffer> m_buffers;

    //images&imageViews
    VkImage textureImage;
    VkImage colorImage;
    VkImage depthImage;
    VkImage shadowMapImage;
    //std::unordered_map<std::string ,VkImage> m_images;

    VkImageView colorImageView;
    VkImageView textureImageView;
    VkImageView depthImageView;
    VkImageView shadowMapImageView;
    //std::unordered_map<std::string ,VkImageView> m_imageViews;

    std::vector<Vertex> vertices = {};
    std::vector<Vertex> vertices2 = {};

    std::vector<uint32_t> indies = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
    };

    std::vector<vertexPositionView> position_views = {};
    std::vector<vertexPositionView> position_views2 = {};
public:
    std::vector<Vertex> quad_vertices = {};
};

#endif //PHY_SIM_VK_RESOURCE_MANAGER_H
