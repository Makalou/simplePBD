#pragma once
#define GLFW_INCLUDE_VULKAN
#include <stdexcept>
#include <GLFW/glfw3.h>
#include "win_config.h"

class VulkanContext;

class Application {
public:
	void run() {
		init_window();
		init_vulkan();
        init_gui();
		mainloop();
		cleanup();
	}
	void setFrameBufferResized(bool flag) {
		frameBufferResized = flag;
	}
private:
	void init_window();
	void init_vulkan();
    void init_gui();
	void mainloop();
	void cleanup();
    void updateGUI();
    void renderGUI(VkCommandBuffer commandBuffer,uint32_t index);
	void drawFrame();
private:
	GLFWwindow* window;
	VulkanContext* vk_context;
	bool frameBufferResized=false;
};