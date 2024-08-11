#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Window.h"
#include <cassert>

#define GLM_FORCE_RADIANS
#define GLM_FROCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace engine {
    class Renderer {
    public:
        std::unique_ptr<SwapChain> m_SwapChain;
        std::vector<VkCommandBuffer> m_CommandBuffers;

    private:
        Window &m_Window;
        Device &m_Device;


        uint32_t m_CurrentImageIndex{0};
        uint32_t m_CurrentFrameIndex{0};
        bool m_IsFramStarted{false};

        glm::vec3 mClearColor;
        VkExtent2D m_extent;

    public:
        Renderer(Window &window, Device &device, const VkExtent2D &extent);

        ~Renderer();

        Renderer(const Renderer &) = delete;

        Renderer &operator=(const Renderer &) = delete;

        void SetClearColor(const glm::vec3 &color) { mClearColor = color; }
        [[nodiscard]] glm::vec3 GetClearColor() const { return mClearColor; }

        [[nodiscard]] VkRenderPass GetSwapChainRenderPass() const {
            return m_SwapChain->getRenderPass();
        }

        [[nodiscard]] float GetAspectRatio() const {
//            return m_SwapChain->extentAspectRatio();
            return static_cast<float>(m_extent.width) /
                   static_cast<float>(m_extent.height);
        }

        [[nodiscard]] bool IsFrameInProgress() const { return m_IsFramStarted; }

        [[nodiscard]] VkCommandBuffer GetCurrentCommandBuffer() const {
            assert(m_IsFramStarted && "Cannot get commandBuffer when frame not in progress");
            return m_CommandBuffers[m_CurrentFrameIndex];
        }

        [[nodiscard]] uint32_t GetFrameIndex() const {
            assert(m_IsFramStarted && "Cannot get frame index when frame not in progress");
            return m_CurrentFrameIndex;
        }

        [[nodiscard]] uint32_t GetImageCount() const {return m_SwapChain->imageCount();}

        std::shared_ptr<Texture> GetTexture();

        VkCommandBuffer BeginFrame();

        void EndFrame();

        void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

        void EndSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

        void RenderImGui();

        void SetExtent(const VkExtent2D &extent) { m_extent = extent; }
        [[nodiscard]] VkExtent2D Extent() const { return m_extent; }

    private:
        void CreateCommandBuffers();

        void FreeCommandBuffers();

        void RecreateSwapChain();
    };
} // namespace engine
