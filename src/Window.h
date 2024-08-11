#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>
#include <glm/vec2.hpp>
#include "imgui/imgui_impl_vulkan.h"

namespace engine {
    class Window {
    private:
        GLFWwindow *m_window;
        ImGui_ImplVulkanH_Window *mImguiWindow;
        uint16_t mWidth, mHeight;
        bool m_FramebufferResized = false;

        std::string m_WindowName;

    public:
        Window(uint16_t width, uint16_t, const std::string &windowName);

        ~Window();

        Window(const Window &) = delete;

        Window &operator=(const Window &) = delete;

        inline bool shouldClose() { return glfwWindowShouldClose(m_window); }

        [[nodiscard]] inline bool wasWindowResized() const { return m_FramebufferResized; }

        inline void resetWindowResizedFlag() { m_FramebufferResized = false; }

        [[nodiscard]] inline VkExtent2D getExtent() const {
            return {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight)};
        }

        [[nodiscard]] inline GLFWwindow *getGLFWwindow() const { return m_window; }

        uint16_t width() { return mWidth; }
        uint16_t height() { return mHeight; }

        [[nodiscard]] glm::vec2 getCursorPosition() const;
        [[nodiscard]] bool isMouseButtonPressed(int32_t button) const;
        [[nodiscard]] bool isKeyPressed(int32_t key) const;

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width,
                                              int height);

        void initWindow();
    };
} // namespace engine
