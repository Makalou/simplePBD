//
// Created by 王泽远 on 2022/6/30.
//

#ifndef PHY_SIM_VK_DESCRIPTOR_H
#define PHY_SIM_VK_DESCRIPTOR_H
#include "vulkan/vulkan.hpp"
#include <optional>

class DescriptorSetLayoutBuilder{
public:

    explicit DescriptorSetLayoutBuilder(VkDevice device){
        m_device = device;
    }

    std::optional<VkDescriptorSetLayout> build() const{
        VkDescriptorSetLayoutCreateInfo info={
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data()
        };

        VkDescriptorSetLayout descriptorSetLayout={};
        if (vkCreateDescriptorSetLayout(m_device, &info, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            return {};
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        return {descriptorSetLayout};
    }

    DescriptorSetLayoutBuilder& addBinding(VkDescriptorType type,uint32_t count,VkShaderStageFlags flags){
        uint32_t cur_size = bindings.size();
        bindings.push_back({
                                      .binding = cur_size,
                                      .descriptorType = type,
                                      .descriptorCount = count,
                                      .stageFlags = flags,
                                      .pImmutableSamplers = nullptr
        });
        return *this;
    }

    DescriptorSetLayoutBuilder& reset(){
        bindings.clear();
        return *this;
    }
private:
    VkDevice m_device;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};
#endif //PHY_SIM_VK_DESCRIPTOR_H
