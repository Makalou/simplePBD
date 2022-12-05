//
// Created by 王泽远 on 2022/7/1.
//
#include "vk_context.h"
#include <algorithm>
#include "utility.h"
#include "vulkan_image_utils.h"
#include "VkBootstrap.h"
#include "vk_descriptor.h"

void MyContext::init_resource() {
    CommandPoolBuilder commandPoolBuilder{vkb_device};

    commandPool = commandPoolBuilder.setQueue(vkb::QueueType::graphics).build().value();
    commandPoolBuilder.setFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    commandBuffers=commandPool.allocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY,vkb_swapchain.image_count);

    for(int i=0;i<vkb_swapchain.image_count;++i){
        auto pool = commandPoolBuilder.build().value();
        gui_commandPools.push_back(pool);
    }

    for(int i=0;i<vkb_swapchain.image_count;++i){
        auto cb = gui_commandPools[i].allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        gui_commandBuffers.push_back(cb);
    }

    VulkanCommandManager::instance().setDevice(vkb_device);

    createRenderPass();
    createShadowPass();
    createImGuiRenderPass();

    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder{vkb_device};
    auto des_set_layout_ret = descriptorSetLayoutBuilder
            .addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1,VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT).build();
    if(!des_set_layout_ret) std::cerr << "Failed to select Vulkan DescriptorSetLayout" << "\n";
    descriptorSetLayout = des_set_layout_ret.value();
    createDescriptorPool(vkb_swapchain.image_count);

    {
        resourceManager.setDevice(vkb_device);
        resourceManager.createColorResources(vkb_swapchain.image_format, vkb_swapchain.extent.width,
                                             vkb_swapchain.extent.height, msaaSamples);
        resourceManager.createDepthResources(vkb_swapchain.extent.width, vkb_swapchain.extent.height, msaaSamples);
        resourceManager.createShadowMapResource(2 * vkb_swapchain.extent.width, 2 * vkb_swapchain.extent.height);
        loadModel(resourceManager.getVertices(), resourceManager.getIndies(), "../res/model/box.obj");
        createTextureSampler();
        resourceManager.createVertexBuffer();
        resourceManager.updateVertexBuffer();
        resourceManager.updateVertexBuffer2();
        resourceManager.createIndexBuffer();
        resourceManager.createUniformBuffers(vkb_swapchain.image_count);//createUniformBuffers();
        resourceManager.createTextureImage(mipLevels);
        resourceManager.createTextureImageView(mipLevels);
    }

    createDescriptorSets(vkb_swapchain.image_count);

    {
        PipelineBuilder pipelineBuilder{vkb_device};
        pipelineBuilder.setShader(VK_SHADER_STAGE_VERTEX_BIT, "../res/shader/shadow.vert.spv")
                .setVertexInputState().setInputAssemblyState().setDepthStencilState()
                .setViewportState(VkExtent2D{.width = 2 * vkb_swapchain.extent.width,.height = 2 * vkb_swapchain.extent.height})
                .setRasterizationState(0.0f, 0.5f)
                .setMultisampleState(VK_SAMPLE_COUNT_1_BIT).setPipelineLayout(descriptorSetLayout)
                .setColorBlendState().setRenderpass(shadowPass);

        auto pipeline_ret = pipelineBuilder.build();
        if(!pipeline_ret) std::cerr << "Failed to build Vulkan shadow Pipeline" << "\n";
        shadowPipeline = pipeline_ret.value();

        pipelineBuilder.reset()
                .setShader(VK_SHADER_STAGE_VERTEX_BIT, "../res/shader/vert.spv")
                .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "../res/shader/frag.spv")
                .setVertexInputState().setInputAssemblyState().setDepthStencilState()
                .setViewportState(vkb_swapchain.extent).setRasterizationState()
                .setMultisampleState(msaaSamples).setColorBlendState()
                .setPipelineLayout(descriptorSetLayout).setRenderpass(renderPass);

        pipeline_ret = pipelineBuilder.build();
        if(!pipeline_ret) std::cerr << "Failed to build Vulkan forward Pipeline" << "\n";
        graphicsPipeline = pipeline_ret.value();
    }

    createFramebuffers(resourceManager.getColorImageView(),resourceManager.getDepthImageView(),renderPass);
    createGUIFramebuffers(imGuiRenderPass);
    createShadowFrambuffers(resourceManager.getShadowMapView(),shadowPass);
    recordCommandBuffers();
}

void MyContext::createRenderPass() {
    VkAttachmentDescription colorAttachment = {
            .format = vkb_swapchain.image_format,
            .samples = msaaSamples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference colorAttachmentRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment = {
            .format = findDepthFormat(vkb_device.physical_device),
            .samples = msaaSamples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };


    VkAttachmentReference depthAttachmentRef = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription colorAttachmentResolve = {
            .format = vkb_swapchain.image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,//present_KHR
    };

    VkAttachmentReference colorAttachmentResolveRef = {
            .attachment = 2,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};

    VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pDepthStencilAttachment = &depthAttachmentRef,
            .pResolveAttachments = &colorAttachmentResolveRef,
    };

    VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
    };

    VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
    };
    if (vkCreateRenderPass(vkb_device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void MyContext::createShadowPass() {
    VkAttachmentDescription attachment = {
            .format = findDepthFormat(vkb_device.physical_device),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .flags = 0
    };

    VkAttachmentReference depth_ref={
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .flags = 0,
            .pDepthStencilAttachment = &depth_ref
    };

    VkRenderPassCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .attachmentCount = 1,
            .pAttachments = &attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
    };
    if (vkCreateRenderPass(vkb_device, &info, nullptr, &shadowPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow render pass!");
    }
}

void MyContext::createImGuiRenderPass() {
    VkAttachmentDescription attachment ={
            .format = vkb_swapchain.image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference color_attachment = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment
    };

    VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo info={
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
    };

    if (vkCreateRenderPass(vkb_device, &info, nullptr, &imGuiRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui render pass!");
    }

}

void MyContext::createDescriptorPool(uint32_t count) {
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(count);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(2*count);

    VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
            .maxSets = static_cast<uint32_t>(count)
    };

    if (vkCreateDescriptorPool(vkb_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    //create imgui descriptorPool

    VkDescriptorPoolSize imgui_pool_sizes[] ={
            { VK_DESCRIPTOR_TYPE_SAMPLER, count },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, count },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, count },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, count }
    };

    VkDescriptorPoolCreateInfo imgui_poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(count),
            .poolSizeCount = static_cast<uint32_t>(std::size(imgui_pool_sizes)),
            .pPoolSizes = imgui_pool_sizes,
    };

    if (vkCreateDescriptorPool(vkb_device, &imgui_poolInfo, nullptr, &imguiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui descriptor pool!");
    }
}

void MyContext::createDescriptorSets(uint32_t count) {
    std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(count),
            .pSetLayouts = layouts.data()
    };

    descriptorSets.resize(count);

    if (vkAllocateDescriptorSets(vkb_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    for (size_t i = 0; i < count; i++) {
        VkDescriptorBufferInfo bufferInfo = {
                .buffer = resourceManager.getUniformBuffers()[i],//binding uniform buffer
                .offset = 0,
                .range = VK_WHOLE_SIZE
        };
        VkDescriptorImageInfo imageInfo = {
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = resourceManager.getTextureImageView(),
                .sampler = textureSampler//binding sampler
        };
        VkDescriptorImageInfo shadowMapInfo = {
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = resourceManager.getShadowMapView(),
                .sampler = shadowMapSampler
        };

        std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;//layout(binding =0)
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pTexelBufferView = nullptr;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;//layout(binding = 1)
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;//layout(binding = 2)
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &shadowMapInfo;

        vkUpdateDescriptorSets(vkb_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                               nullptr);
    }
}

void MyContext::recordCommandBuffers() {
    auto commandBufferCount = commandBuffers.size();
    for (size_t i = 0; i < commandBufferCount; i++) {

        auto commandBuffer = commandBuffers[i];
        VkCommandBufferBeginInfo beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
        };

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        recordShadowPass(commandBuffer,i);
        recordForwardPass(commandBuffer,i);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void MyContext::recordShadowPass(VkCommandBuffer commandBuffer,uint32_t des_idx) {
    VkClearValue clearValue = {
            .depthStencil.depth = 1.0f,
            .depthStencil.stencil = 0
    };

    VkRenderPassBeginInfo passBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = shadowPass,
            .framebuffer = m_shadow_framebuffers[des_idx],
            .renderArea = {
                    .offset = {0,0},
                    .extent = VkExtent2D{.width = 2 * vkb_swapchain.extent.width,.height = 2 * vkb_swapchain.extent.height}
            },
            .clearValueCount = 1,
            .pClearValues = &clearValue
    };
    vkCmdBeginRenderPass(commandBuffer,&passBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.layout, 0, 1,
                            &descriptorSets[des_idx], 0, nullptr);

    VkBuffer vertexBuffers[] = {resourceManager.getVertexBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, resourceManager.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(resourceManager.getIndies().size()), 1, 0, 0, 0);

    vertexBuffers[0] = resourceManager.getVertexBuffer2();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(resourceManager.getIndies().size()), 1, 0, 0, 0);

    VkBuffer quadVertexBuffers[] = {resourceManager.getQuadVertexBuffer()};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, quadVertexBuffers, offsets);
    vkCmdDraw(commandBuffer,6,1,0,0);
    vkCmdEndRenderPass(commandBuffer);
}

void MyContext::recordForwardPass(VkCommandBuffer commandBuffer, uint32_t des_idx) {

    std::array<VkClearValue, 2> clearValues = {};
    const float a = 0.9f;
    clearValues[0].color = {a, a, a, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderPass,
            .framebuffer = m_framebuffers[des_idx],
            .renderArea.offset = {0, 0},
            .renderArea.extent = vkb_swapchain.extent,
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
    };

    vkCmdBeginRenderPass(commandBuffer, &beginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.layout, 0, 1,
                            &descriptorSets[des_idx], 0, nullptr);

    vkCmdBindIndexBuffer(commandBuffer, resourceManager.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
    VkDeviceSize offsets[] = {0};

    VkBuffer vertexBuffers[] = {resourceManager.getVertexBuffer()};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(resourceManager.getIndies().size()), 1, 0, 0, 0);
    vertexBuffers[0] = resourceManager.getVertexBuffer2();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(resourceManager.getIndies().size()), 1, 0, 0, 0);

    VkBuffer quadVertexBuffers[] = {resourceManager.getQuadVertexBuffer()};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, quadVertexBuffers, offsets);
    vkCmdDraw(commandBuffer,6,1,0,0);
    vkCmdEndRenderPass(commandBuffer);
}

void MyContext::cleanupSwapChain() {

    resourceManager.cleanUpColorResources();
    resourceManager.cleanUpDepthResources();

    destroyAllFrameBuffers();

    //VulkanCommandManager::instance().freeBuffers();
    //VulkanCommandManager::instance().freeGuiCommandBuffers();
    commandPool.freeBuffers(commandBuffers);
    commandPool.destroy();
    for(int i =0;i<gui_commandBuffers.size();++i){
        gui_commandPools[i].freeBuffers(gui_commandBuffers.data()+i,1);
        gui_commandPools[i].destroy();
    }

    graphicsPipeline.destroy(vkb_device);
    shadowPipeline.destroy(vkb_device);

    destroyDeviceObject(vkDestroyRenderPass, renderPass);
    destroyDeviceObject(vkDestroyRenderPass, imGuiRenderPass);

    //swapChainWrapper.destoryImageViews();
    vkb_swapchain.destroy_image_views(vkb_swapchain.get_image_views().value());

    resourceManager.cleanUpUniformBuffers();

    destroyDeviceObject(vkDestroyDescriptorPool, descriptorPool);
    destroyDeviceObject(vkDestroyDescriptorPool, imguiDescriptorPool);

    //swapChainWrapper.destory();
    destroy_swapchain(vkb_swapchain);
}

void MyContext::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = static_cast<float>(mipLevels),
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
    };

    if (vkCreateSampler(vkb_device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    VkSamplerCreateInfo shadowMapSamplerInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .anisotropyEnable = false,
            .maxAnisotropy = 1.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = false,
            .compareEnable = false,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = 100.0f,
    };

    if (vkCreateSampler(vkb_device, &shadowMapSamplerInfo, nullptr, &shadowMapSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadowMap sampler!");
    }

}

void MyContext::recreateSwapChain() {

    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vkb_device);

    cleanupSwapChain();
    createRenderPass();
    createImGuiRenderPass();

    resourceManager.createColorResources(vkb_swapchain.image_format, vkb_swapchain.extent.width, vkb_swapchain.extent.height, msaaSamples);
    resourceManager.createDepthResources(vkb_swapchain.extent.width, vkb_swapchain.extent.height, msaaSamples);

    createFramebuffers(resourceManager.getColorImageView(),resourceManager.getDepthImageView(),renderPass);
    createGUIFramebuffers(imGuiRenderPass);
    createShadowFrambuffers(resourceManager.getShadowMapView(),shadowPass);
    resourceManager.createUniformBuffers(vkb_swapchain.image_count);

    createDescriptorPool(vkb_swapchain.image_count);
    createDescriptorSets(vkb_swapchain.image_count);

    //VulkanCommandManager::instance().createCommandBuffers(vkb_swapchain.image_count);
    recordCommandBuffers();
}

void MyContext::clean_resource() {
    cleanupSwapChain();
    destroyDeviceObject(vkDestroySampler, textureSampler);
    destroyDeviceObject(vkDestroyDescriptorSetLayout, descriptorSetLayout);
    resourceManager.cleanUp();
}