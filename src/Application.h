#pragma once

#include "Device.h"
#include "Pipeline.h"
#include "Window.h"
#include "Renderer.h"
#include "Buffer.h"
#include "descriptors/DescriptorPool.h"
#include "Components.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FROCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <entt/entt.hpp>

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace engine {

    struct ImGuiImage {
        VkDescriptorSet m_image;
        ImVec2 m_size;
    };

    class Application {
    private:
        static constexpr uint16_t WIDTH = 1920;
        static constexpr uint16_t HEIGHT = 1920;
        Window mWindow{WIDTH, HEIGHT, "App"};
        Device mDevice{mWindow};
        Renderer mRenderer{mWindow, mDevice, {100, 100}};

        // Declaration order matters!!!!!!
        std::unique_ptr<DescriptorPool> mGlobalPool{};

        glm::vec3 m_backgroundColor;
        glm::vec4 m_snakeColor;

        entt::registry m_registry;

    public:
        Application();
        ~Application();
        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        void run();

    private:
        void Update();
        void Render();

        static float frand(float min, float max);
    };
} // namespace engine
