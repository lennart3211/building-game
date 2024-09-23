#include "Window.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace engine {
    Window::Window(uint16_t width, uint16_t height, const std::string &windowName)
            : mWidth(width), mHeight(height), m_WindowName(windowName) {
        initWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void Window::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window =
                glfwCreateWindow(mWidth, mHeight, &m_WindowName[0], nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width,
                                           int height) {
        auto newWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        newWindow->m_FramebufferResized = true;
        newWindow->mWidth = width;
        newWindow->mHeight = height;
    }

    glm::vec2 Window::getCursorPosition() const {
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);

        int width, height;
        glfwGetWindowSize(m_window, &width, &height);

        float normalizedX = (2.0f * (float) x) / width - 1.0f;
        float normalizedY = 1.0f - (2.0f * (float) y) / height; // Flip Y-axis

        normalizedY *= -1;

        return {normalizedX, normalizedY};
    }

    bool Window::isMouseButtonPressed(int32_t button) const {
        return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
    }

    bool Window::isKeyPressed(int32_t key) const {
        return glfwGetKey(m_window, key) == GLFW_PRESS;
    }
    bool Window::minimized() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        return (width == 0 && height == 0); }

    } // namespace engine
