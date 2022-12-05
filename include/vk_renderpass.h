//
// Created by 王泽远 on 2022/6/28.
//

#ifndef PHY_SIM_VK_RENDERPASS_H
#define PHY_SIM_VK_RENDERPASS_H

#include "optional"
#include "vulkan/vulkan.hpp"

class VulkanRenderPass{
public:
    void bindPipeline();
    void bindDescriptorSets();
    void record();

    operator VkRenderPass() const;
};

class RenderPassBuilder{
public:
    std::optional<VulkanRenderPass> build() const;
    void reset() const;
    void addAttachment();
private:
    struct RenderPassInfo{

    }info;
    std::vector<VkAttachmentReference> att;
};
#endif //PHY_SIM_VK_RENDERPASS_H
