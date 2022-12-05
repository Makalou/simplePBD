#pragma once

#include <fstream>
#include <vector>
#include "vertex.h"

std::vector<char> readFile(const std::string& filename);

void loadModel(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const std::string path);

/*
void show_available_extensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extesions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extesions.data());

    std::cout << "available extensions:" << std::endl;
    for (const auto &ext: extesions) {
        std::cout << "\t" << ext.extensionName << std::endl;
    }

}

void show_glfwextesions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::cout << "glfwExtensionNames:" << std::endl;
    for (int i = 0; i < glfwExtensionCount; i++) {
        std::cout << "\t" << *(glfwExtensions + i) << std::endl;
    }
}
*/
