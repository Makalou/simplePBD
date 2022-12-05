//
// Created by 王泽远 on 2022/5/26.
//

#ifndef PHY_SIM_VK_INSTANCE_H
#define PHY_SIM_VK_INSTANCE_H

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
#include <iostream>

const std::vector<const char*>validationLayers = { "VK_LAYER_KHRONOS_validation" };

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
        VkDebugUtilsMessageTypeFlagsEXT msgType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
    std::string msg(pCallbackData->pMessage);
    auto p = msg.find("]");
    auto type=msg.substr(0,++p);
    auto ss = msg.substr(p);
    p = ss.find("|");
    auto obj = ss.substr(0, p);
    ss=ss.substr(++p);
    p = ss.find("|");
    auto msgID= ss.substr(0, p);
    ss = ss.substr(++p);

    std::cout << type << std::endl;
    std::cout << obj << std::endl;
    std::cout << msgID << std::endl;
    std::cout <<ss<< std::endl;
    std::cout << "\n*****************************************************************************************\n" << std::endl;
    return VK_FALSE;
}

class VulkanInstance
{
public:
    VulkanInstance()=default;
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VkInstance get_vk_instance()const {
        return instance;
    }

    void create(const std::string& appName,const std::string& engineName){
        VkApplicationInfo appInfo = {};
        fill_appinfo(&appInfo,appName,engineName);
        VkInstanceCreateInfo createInfo = {};

        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = static_cast<const char**>(extensions.data());

        std::cout<<"enable extensions"<<"\n";
        for(const auto & ext:extensions){
            std::cout<<"\t"<<ext<<"\n";
        }

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames=validationLayers.data();

        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        auto result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void destroy(){
        destroyDebugUtilsMessengerEXT(instance,debugMessenger, nullptr);
        vkDestroyInstance(instance,nullptr);
    }

    void enableDebug(){
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;

        if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
private:

    VkInstance instance{};

    VkDebugUtilsMessengerEXT debugMessenger{};

    static void fill_appinfo(VkApplicationInfo* pAppInfo,const std::string& appName,const std::string& engineName)
    {
        pAppInfo->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        pAppInfo->pApplicationName = appName.c_str();
        pAppInfo->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        pAppInfo->pEngineName = engineName.c_str();
        pAppInfo->engineVersion = VK_MAKE_VERSION(1, 0, 0);
        pAppInfo->apiVersion = VK_API_VERSION_1_0;// 1.0 1.1 1.2
    }

    static std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        return extensions;
    }

    static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void destroyDebugUtilsMessengerEXT(VkInstance instance,VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator){
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, messenger, pAllocator);
        }
    }
};


#endif //PHY_SIM_VK_INSTANCE_H
