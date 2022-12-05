#include "Application.h"
#include "vk_context.h"
#include "camera.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include <queue>

static void frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->setFrameBufferResized(true);
    std::cout<<"window resize: ["<<width<<","<<height<<"]\n";
}

static Camera *pCamera = new Camera();

float deltaTime = 0.0f;
float lastFrame = 0.0f;

static void cursorPosCallback(GLFWwindow *window, double x, double y) {
    static double old_x = -1;
    static double old_y = -1;
    if (old_x == -1 || old_y == -1) {
        old_x = x;
        old_y = y;
        return;
    }
    auto x_offset = x - old_x;
    old_x = x;
    auto y_offset = old_y - y;
    old_y = y;
    pCamera->deflect(x_offset, y_offset);
    //pCamera->debug();
}

auto currentMouseVisible = GLFW_CURSOR_NORMAL;

static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_W:
            pCamera->transfrom(CameraMovement::FORWARD, deltaTime);
            break;
        case GLFW_KEY_S:
            pCamera->transfrom(CameraMovement::BACKWARD, deltaTime);
            break;
        case GLFW_KEY_A:
            pCamera->transfrom(CameraMovement::LEFT, deltaTime);
            break;
        case GLFW_KEY_D:
            pCamera->transfrom(CameraMovement::RIGHT, deltaTime);
            break;
        case GLFW_KEY_H:
            if (currentMouseVisible == GLFW_CURSOR_NORMAL) {
                currentMouseVisible = GLFW_CURSOR_DISABLED;
                glfwSetCursorPosCallback(window, cursorPosCallback);
            } else {
                currentMouseVisible = GLFW_CURSOR_NORMAL;
                glfwSetCursorPosCallback(window, nullptr);
            }
            glfwSetInputMode(window, GLFW_CURSOR, currentMouseVisible);
            break;

        default:
            break;
    }
}

static void scrollCallback(GLFWwindow *window, double x_offset, double y_offset) {
    pCamera->zoom(y_offset);
}

static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

    }
}

void Application::init_window() {
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("glfwinit failed!");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(WIN_WIDTH, WIN_HIGHT, WIN_NAME, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    Camera::registry("camera1", pCamera);
}

void Application::init_vulkan() {
    vk_context = new MyContext;
    vk_context->init(window);
}

//glm::vec3 v = {};
const glm::vec3 gravity = {0.f, -9.8f, 0.f};
const glm::vec3 contact_force = glm::vec3{0.f, 2000.0f, 0.f};
//const float damping = 0.999f;
const float damping = 1;

const float substep = 24;

const float k1 = 1.0f;
const float k2 = 0.005f;
const float k3 = 0.001f;
const float stiffness = 1;

using Face = std::tuple<size_t ,size_t ,size_t >;

std::vector<Face> faces={
        {1,5,7},{1,7,3},
        {4,3,7},{4,7,8},
        {8,7,5},{8,5,6},
        {6,2,4},{6,4,8},
        {2,1,3},{2,3,4},
        {6,5,1},{6,1,2},
};

inline
bool intersect(const glm::vec3& o,const glm::vec3 & d, const glm::vec3 & v1,const glm::vec3 & v2,const glm::vec3 & v3,float & t){
    return false;
}

std::vector<glm::vec3> v_s;
std::vector<glm::vec3> v_s2;

std::vector<std::tuple<int,int,float>> lengths;
std::vector<std::tuple<int,int,float>> lengths2;

std::vector<std::tuple<size_t ,int,size_t ,int,int,int>> collisions;

std::vector<float> v_y;
std::vector<float> energy;

size_t vex_num;

bool paused = true;
bool collide = false;
bool rota = false;

void Application::mainloop() {
    const auto scale = 0.2f;
    auto m = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    m = glm::rotate(m,glm::radians(-0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::rotate(m,glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    auto vertex_positions = dynamic_cast<MyContext*>(vk_context)->resourceManager.getVertexPositionView();
    auto vertex_positions2 =  dynamic_cast<MyContext*>(vk_context)->resourceManager.getVertexPositionView2();

    auto p = std::vector<glm::vec3> {};
    auto p2 = std::vector<glm::vec3> {};

    for (auto & pos: vertex_positions) {
        pos = glm::vec3(m*glm::vec4(*pos,1.0f));
        pos += glm::vec3 {0,10,0};
        p.push_back(*pos);
    }

    for(auto & pos: vertex_positions2){
        pos = glm::vec3(m*glm::vec4(*pos,1.0f));
        pos += glm::vec3 {0,5,0};
        p2.push_back(*pos);
    }

    dynamic_cast<MyContext*>(vk_context)->resourceManager.updateVertexBuffer();
    dynamic_cast<MyContext*>(vk_context)->resourceManager.updateVertexBuffer2();

    v_s.resize(vertex_positions.size());
    v_s2.resize(vertex_positions2.size());

    for(auto & vv: v_s){
        vv = {5,0,0};
    }

    for(auto & vv: v_s2){
        vv = {0,0,5};
    }

    vex_num = p.size();

    for(int i = 0;i<p.size();++i){
        for(int j = i+1;j<p.size();++j){
            auto c = glm::length(p[i]-p[j]);
            lengths.emplace_back(i,j,c);
        }
    }

    for(int i = 0;i<p2.size();++i){
        for(int j = i+1;j<p2.size();++j){
            auto c = glm::length(p2[i]-p2[j]);
            lengths2.emplace_back(i,j,c);
        }
    }

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        auto currentFrame = (float) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if(!paused) {
            for(auto &vv:v_s){
                vv+=gravity*deltaTime;
                vv*=damping;
            }
            for(auto &vv:v_s2){
                vv+=gravity*deltaTime;
                vv*=damping;
            }

            //update position
            for (int i=0;i<vex_num;++i) {
                auto & vex = vertex_positions[i];
                p[i] = *vex+deltaTime*v_s[i];

                auto & vex2 = vertex_positions2[i];
                p2[i] = *vex2+deltaTime*v_s2[i];
            }

            //handle constraints
            auto max_distance = 0.0f;
            auto center_high = std::numeric_limits<float>::max();

            for(int s = 0;s<substep;++s)
            {
#define USE_BOUNDARY_COLLISION_CONSTRAINT
#ifdef USE_BOUNDARY_COLLISION_CONSTRAINT
                //collision constraints
                for (int i=0;i<vex_num;++i) {
                    //max_distance = std::max(max_distance, -vex.pos.y);
                    //center_high = std::min(center_high, vex.pos.y);

                    auto & vex = p[i];
                    if(vex.y<0&&v_s[i].y<0) vex.y *=-0.9f;
                    if(vex.x<-2&&v_s[i].x<0) vex.x = -4-vex.x;
                    if(vex.x>2&&v_s[i].x>0) vex.x = 4-vex.x;
                    if(vex.z<-2&&v_s[i].z<0) vex.z = -4-vex.z;
                    if(vex.z>2&&v_s[i].z>0) vex.z = 4-vex.z;
                    //vex.y = std::clamp(vex.y,.0f,std::numeric_limits<float>::max());
                    //vex.x = std::clamp(vex.x,-2.0f,2.0f);
                    //vex.z = std::clamp(vex.z,-2.0f,2.0f);

                    auto & vex2 = p2[i];
                    if(vex2.y<0&&v_s2[i].y<0) vex2.y *=-0.9f;
                    if(vex2.x<-2&&v_s2[i].x<0) vex2.x = -4-vex2.x;
                    if(vex2.x>2&&v_s2[i].x>0) vex2.x = 4-vex2.x;
                    if(vex2.z<-2&&v_s2[i].z<0) vex2.z = -4-vex2.z;
                    if(vex2.z>2&&v_s2[i].z>0) vex2.z = 4-vex2.z;
                    //vex2.y = std::clamp(vex2.y,.0f,std::numeric_limits<float>::max());
                    //vex2.x = std::clamp(vex2.x,-2.0f,2.0f);
                    //vex2.z = std::clamp(vex2.z,-2.0f,2.0f);
                }
#endif
#define USE_BODY_COLLISION_CONSTRAINT
#ifdef USE_BODY_COLLISION_CONSTRAINT
                glm::vec3 center;
                for (int i=0;i<vex_num;++i) {
                    center += p[i];
                }
                center/=vex_num;

                for(int i = 0;i<vex_num;++i){
                    auto & vex2 = p2[i];
                    auto cc = vex2-center;
                    auto cl = glm::length(cc);
                    const float c = 0.75/2.0f;
                    collide = cl<c;
                    if(collide){
                        glm::vec3 n = glm::normalize(cc);
                        auto delta = stiffness*0.5f*(c-cl)*n;
                        for (int i=0;i<vex_num;++i) {
                            auto & vex = p[i];
                            vex -= delta;
                        }
                        vex2 += delta;
                    }
                }

                for(int i =0;i<vex_num;++i){
                    for(const auto & f:faces){
                        float t;
                        auto v1 = *vertex_positions2[std::get<0>(f)];
                        auto v2 = *vertex_positions2[std::get<1>(f)];
                        auto v3 = *vertex_positions2[std::get<2>(f)];

                        if(intersect(p[i],*vertex_positions[i],v1,v2,v3,t)){
                            auto q = *vertex_positions[i]+t*(p[i]-*vertex_positions[i]);
                        }
                    }
                }

#endif
                //length constraints
#define USE_LENGTH_CONSTRAINT
#ifdef USE_LENGTH_CONSTRAINT
                for(auto constraint = lengths.begin();constraint!=lengths.end();++constraint){
                        auto i = std::get<0>(*constraint); auto j = std::get<1>(*constraint);
                        auto d = std::get<2>(*constraint);
                        auto cv = p[i]-p[j];
                        float l = glm::length(cv);
                        glm::vec3 n = glm::normalize(cv);
                        auto deltaP = stiffness*0.5f*(l-d)*n;
                        p[i]-=deltaP;
                        p[j]+=deltaP;

                }
                for(auto constraint = lengths2.begin();constraint!=lengths2.end();++constraint){
                    auto i = std::get<0>(*constraint); auto j = std::get<1>(*constraint);
                    auto d = std::get<2>(*constraint);
                    auto cv = p2[i]-p2[j];
                    float l = glm::length(cv);
                    glm::vec3 n = glm::normalize(cv);
                    auto deltaP = stiffness*0.5f*(l-d)*n;
                    p2[i]-=deltaP;
                    p2[j]+=deltaP;
                }
#endif
            }

            //update velocity
            for (int i=0;i<vex_num;++i) {
                auto & vex = vertex_positions[i];
                v_s[i] = (p[i] - *vex)/deltaTime;

                auto & vex2 = vertex_positions2[i];
                v_s2[i] = (p2[i] - *vex2)/deltaTime;
            }

            //update position
            for (int i=0;i<vex_num;++i) {
                auto & vex = vertex_positions[i];
                vex = p[i];

                auto & vex2 = vertex_positions2[i];
                vex2 = p2[i];
            }

            dynamic_cast<MyContext*>(vk_context)->resourceManager.updateVertexBuffer();
            dynamic_cast<MyContext*>(vk_context)->resourceManager.updateVertexBuffer2();
        }
        updateGUI();
        drawFrame();
    }
    vkDeviceWaitIdle(vk_context->vkb_device);
}

void Application::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    vk_context->cleanup();
    delete vk_context;
    glfwDestroyWindow(window);
    glfwTerminate();
}

size_t currentFrame = 0;
static int count = 0;

void Application::drawFrame() {
    auto *data = ImGui::GetDrawData();

    vkWaitForFences(vk_context->vkb_device, 1, &vk_context->inFlightFences[currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
    uint32_t imageIndex;
    auto aquiresult = vkAcquireNextImageKHR(vk_context->vkb_device, vk_context->get_swapChain(),
                                            std::numeric_limits<uint64_t>::max(),
                                            vk_context->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
                                            &imageIndex);

    if (aquiresult == VK_ERROR_OUT_OF_DATE_KHR || frameBufferResized) {
        setFrameBufferResized(false);
        vk_context->recreateSwapChain();
        return;
    } else if (aquiresult != VK_SUCCESS && aquiresult != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    dynamic_cast<MyContext*>(vk_context)->resourceManager.updateUniformBuffer(imageIndex, vk_context->vkb_swapchain.extent.width /
                                                                (float) vk_context->vkb_swapchain.extent.height, rota);
    VkSemaphore waitSemaphores[] = {vk_context->imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    auto command = dynamic_cast<MyContext*>(vk_context)->commandBuffers[imageIndex];
    auto gui_command = dynamic_cast<MyContext*>(vk_context)->gui_commandBuffers[imageIndex];

    renderGUI(gui_command, imageIndex);

    VkSemaphore signalSemaphores[] = {vk_context->renderFinishedSemaphores[currentFrame]};

    VkCommandBuffer commandBuffers[] = {command, gui_command};

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = std::size(commandBuffers),
            .pCommandBuffers = commandBuffers,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
    };

    //render queue -> producer
    //present queue -> consumer
    //swapchain -> mailbox
    vkResetFences(vk_context->vkb_device, 1, &vk_context->inFlightFences[currentFrame]);
    if (vkQueueSubmit(vk_context->vkb_device.get_queue(vkb::QueueType::graphics).value(), 1, &submitInfo,
                      vk_context->inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {vk_context->get_swapChain()};
    VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex
    };

    auto qp_result = vkQueuePresentKHR(vk_context->vkb_device.get_queue(vkb::QueueType::present).value(), &presentInfo);
    if (qp_result == VK_ERROR_OUT_OF_DATE_KHR || qp_result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
        setFrameBufferResized(false);
        vk_context->recreateSwapChain();
    } else if (qp_result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    //vkQueueWaitIdle(vk_context->presentQueue);

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}

void Application::init_gui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = vk_context->vkb_inst,//vk_context->vk_instance_wrapper.get_vk_instance(),
            .PhysicalDevice = vk_context->vkb_device.physical_device,
            .Device = vk_context->getDevice(),
            //todo .quefamily
            .Queue = vk_context->vkb_device.get_queue(vkb::QueueType::graphics).value(),
            .PipelineCache = VK_NULL_HANDLE,
            .DescriptorPool = dynamic_cast<MyContext*>(vk_context)->imguiDescriptorPool,
            .MinImageCount = static_cast<uint32_t>(vk_context->vkb_swapchain.image_count),
            .ImageCount = static_cast<uint32_t>(vk_context->vkb_swapchain.image_count),
            .Allocator = VK_NULL_HANDLE,
            .CheckVkResultFn = nullptr
    };
    ImGui_ImplVulkan_Init(&init_info, dynamic_cast<MyContext*>(vk_context)->imGuiRenderPass);

    auto command_buffer = VulkanCommandManager::instance().beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    VulkanCommandManager::instance().endandSubmitSingleTimeCommands(command_buffer);

}

void Application::updateGUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();
    {
        ImGui::Begin("Graphs");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Checkbox("View rotation",&rota);

        ImGui::Checkbox("Simulation Paused",&paused);

        ImGui::Text("Vertices number: %zu",vex_num);

        //if(ImGui::Button("New Cube"))

        ImGui::Text("Constraints1 number: %zu",lengths.size());
        ImGui::Text("Constraints2 number: %zu",lengths2.size());

        ImGui::Text("Collide: %d",collide);
        // Fill an array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
        // and the sizeof() of your structure in the "stride" parameter.

        /*
        static double refresh_time = 0.0;
        if (!animate || refresh_time == 0.0)
            refresh_time = ImGui::GetTime();
        while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
        {
            static float phase = 0.0f;
            values[values_offset] = cosf(phase);
            values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
            phase += 0.10f * values_offset;
            refresh_time += 1.0f / 60.0f;
        }
         */
        if (v_y.size()>10000)
            v_y.erase(v_y.begin());
        // Plots can display overlay texts
        // (in this example, we will display an average value)
        /*
        {
            float average = 0.0f;
            for (int n = 0; n < v_y.size(); n++)
                average += values[n];
            average /= (float)IM_ARRAYSIZE(values);
            char overlay[32];
            sprintf(overlay, "avg %f", average);
        }
         */

        ImGui::PlotLines("Velocity", v_y.data(), v_y.size(), 0, nullptr, -10.0f, 10.0f, ImVec2(0, 80.0f));

        if(energy.size()>10000)
            energy.erase(energy.begin());

        ImGui::PlotLines("Energy", energy.data(), energy.size(), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80.0f));

        ImGui::End();
    }

    ImGui::Render();
}

void Application::renderGUI(VkCommandBuffer commandBuffer, uint32_t index) {
    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = dynamic_cast<MyContext*>(vk_context)->imGuiRenderPass;
    info.framebuffer = dynamic_cast<MyContext*>(vk_context)->m_gui_framebuffers[index];
    info.renderArea.extent.width = vk_context->vkb_swapchain.extent.width;
    info.renderArea.extent.height = vk_context->vkb_swapchain.extent.height;
    vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
}
