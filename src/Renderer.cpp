#include "Renderer.h"

#include <array>
#include <cassert>
#include <stdexcept>

namespace engine {
    Renderer::Renderer(Window &window, Device &device, const VkExtent2D &extent)
    : m_Window(window), m_Device(device), m_extent(extent) {
        RecreateSwapChain();
        CreateCommandBuffers();
    }

    Renderer::~Renderer() { FreeCommandBuffers(); }

    void Renderer::RecreateSwapChain() {
//        auto extent = m_Window.getExtent();

//        while (m_extent.width == 0 || m_extent.height == 0) {
//            m_extent = m_Window.getExtent();
//            glfwWaitEvents();
//        }

        vkDeviceWaitIdle(m_Device.device());

        if (m_SwapChain == nullptr) {
            m_SwapChain = std::make_unique<SwapChain>(m_Device, m_extent);
        } else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<SwapChain>(m_Device, m_extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*m_SwapChain)) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void Renderer::CreateCommandBuffers() {
        m_CommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_Device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        if (vkAllocateCommandBuffers(m_Device.device(), &allocInfo,
                                     m_CommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers");
        }
    }

    void Renderer::FreeCommandBuffers() {
        vkFreeCommandBuffers(m_Device.device(), m_Device.getCommandPool(),
                             static_cast<uint32_t>(m_CommandBuffers.size()),
                             m_CommandBuffers.data());

        m_CommandBuffers.clear();
    }

    VkCommandBuffer Renderer::BeginFrame() {
        assert(!m_IsFramStarted && "Can't call BeginFrame while already in progress");

        auto result = m_SwapChain->acquireNextImage(&m_CurrentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        m_IsFramStarted = true;

        auto commandBuffer = GetCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }
        return commandBuffer;
    }

    void Renderer::EndFrame() {
        assert(m_IsFramStarted && "Can't call EndFrame while frame is not in progress");
        auto commandBuffer = GetCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }

        auto result = m_SwapChain->submitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            m_Window.wasWindowResized()) {
            m_Window.resetWindowResizedFlag();
            RecreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image");
        }

        m_IsFramStarted = false;
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer) const {
        assert(m_IsFramStarted && "Can't call BeginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == GetCurrentCommandBuffer() &&
               "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->getRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {mClearColor.r, mClearColor.g, mClearColor.b, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_SwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer) const {
        assert(m_IsFramStarted && "Can't call EndSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == GetCurrentCommandBuffer() &&
               "Can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);

        m_SwapChain->copySwapChainImageToImGuiTexture(commandBuffer, m_CurrentImageIndex);
    }

    std::shared_ptr<Texture> Renderer::GetTexture() {
        return m_SwapChain->getTexture();
    }

    void Renderer::RenderImGui() {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();

        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = m_SwapChain->getRenderPass();
        info.framebuffer = m_SwapChain->getFrameBuffer(m_CurrentImageIndex);
        info.renderArea.extent = m_SwapChain->getSwapChainExtent();

        // Provide clear values for color and depth-stencil attachments
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f}; // Clear color (black with full opacity)
        clearValues[1].depthStencil = {1.0f, 0}; // Clear depth to 1.0f and stencil to 0

        info.clearValueCount = static_cast<uint32_t>(clearValues.size());
        info.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

        vkCmdEndRenderPass(commandBuffer);
    }

} // namespace engine
