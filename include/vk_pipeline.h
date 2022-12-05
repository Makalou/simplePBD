//
// Created by 王泽远 on 2022/6/29.
//

#ifndef PHY_SIM_VK_PIPELINE_H
#define PHY_SIM_VK_PIPELINE_H

#include "vulkan/vulkan.hpp"
#include <optional>

class VulkanPipeline{
    operator VkPipeline() const;
};

class PipelineBuilder{
    std::optional<VulkanPipeline> build() const;
};

#endif //PHY_SIM_VK_PIPELINE_H
