//
// Created by 王泽远 on 2022/5/26.
//
#include "vk_resource_manager.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

void VulkanResourceManager::createVertexBuffer() {

    for(auto &v : vertices){
        bool find = false;
        for(auto & view : position_views){
            if(view == v.pos){
                view.reg(&v.pos);
                find = true;
                break;
            }
        }
        if(!find){
            vertexPositionView view;
            view.reg(&v.pos);
            position_views.push_back(view);
        }
    }

    vertices2.resize(vertices.size());
    std::copy(vertices.begin(),vertices.end(),vertices2.begin());

    for(auto &v : vertices2){
        bool find = false;
        for(auto & view : position_views2){
            if(view == v.pos){
                view.reg(&v.pos);
                find = true;
                break;
            }
        }
        if(!find){
            vertexPositionView view;
            view.reg(&v.pos);
            position_views2.push_back(view);
        }
    }

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer2, vertexBufferMemory);

    const float quad_half_length = 2.0f;
    quad_vertices.push_back(Vertex{.pos = {-quad_half_length,0.0f,-quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
    quad_vertices.push_back(Vertex{.pos = {-quad_half_length,0.0f,quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
    quad_vertices.push_back(Vertex{.pos = {quad_half_length,0.0f,-quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
    quad_vertices.push_back(Vertex{.pos = {quad_half_length,0.0f,-quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
    quad_vertices.push_back(Vertex{.pos = {-quad_half_length,0.0f,quad_half_length}, .normal = {0.0f,1.0f,0.0f}});
    quad_vertices.push_back(Vertex{.pos = {quad_half_length,0.0f,quad_half_length}, .normal = {0.0f,1.0f,0.0f}});

    bufferSize = sizeof(quad_vertices[0]) * quad_vertices.size();
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadVertexBuffer, quadVertexBufferMemory);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, quad_vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    copyBuffer(stagingBuffer, quadVertexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanResourceManager::updateVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanResourceManager::updateVertexBuffer2() {
    VkDeviceSize bufferSize = sizeof(vertices2[0]) * vertices2.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices2.data(), (size_t) bufferSize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer2, bufferSize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanResourceManager::createIndexBuffer() {
    VkDeviceSize buffersize = sizeof(indies[0]) * indies.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, buffersize, 0, &data);
    memcpy(data, indies.data(), (size_t) buffersize);
    vkUnmapMemory(m_device, stagingBufferMemory);

    createBuffer(buffersize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, buffersize);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanResourceManager::createUniformBuffers(size_t ub_size) {
    VkDeviceSize buffersize = sizeof(UniformBufferObject);
    uniformBuffers.resize(ub_size);
    uniformBuffersMemory.resize(ub_size);
    for (size_t i = 0; i < ub_size; i++) {
        createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i],
                     uniformBuffersMemory[i]);
    }
}

void VulkanResourceManager::updateUniformBuffer(uint32_t currentImage, float aspect,bool rotate) {
    UniformBufferObject ubo = {};
    UniformBufferObject::update(ubo, aspect,rotate);
    void *data;
    vkMapMemory(m_device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_device, uniformBuffersMemory[currentImage]);
}

void VulkanResourceManager::createTextureImage(uint32_t &mipLevels) {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load("../res/texture/eva01/textura.png", &texWidth, &texHeight, &texChannels,
                                STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load image!");
    }

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texHeight, texWidth)))) + 1;

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);
    stbi_image_free(pixels);

    createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                textureImage, textureImageMemory);
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    copyBufferToImage(stagingBuffer, textureImage, texWidth, texHeight);
    generateMipmaps(textureImage, texWidth, texHeight, mipLevels);
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanResourceManager::createTextureImageView(uint32_t mipLevels) {
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
                                       mipLevels);
}

void VulkanResourceManager::createColorResources(VkFormat swapChainImageFormat, uint32_t width, uint32_t height,
                                                 VkSampleCountFlagBits msaaSamples) {
    VkFormat colorFormat = swapChainImageFormat;

    createImage(width, height, 1, msaaSamples, colorFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                colorImage, colorImageMemory);
    colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
}

void VulkanResourceManager::createDepthResources(uint32_t width, uint32_t height,VkSampleCountFlagBits msaaSamples) {
    VkFormat depthFormat = findDepthFormat();
    createImage(width, height, 1, msaaSamples,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void VulkanResourceManager::createShadowMapResource(uint32_t width,uint32_t height) {
    VkFormat format = findDepthFormat();
    createImage(width,height,1,VK_SAMPLE_COUNT_1_BIT,format,
                VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,shadowMapImage,shadowMapMemory);
    shadowMapImageView = createImageView(shadowMapImage,format,VK_IMAGE_ASPECT_DEPTH_BIT,1);
    transitionImageLayout(shadowMapImage,format,VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,1);
}
