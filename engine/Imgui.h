#pragma once

#include "Device.h"
#include "Window.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <stdexcept>

namespace engine {
    static void check_vk_result(VkResult err) {
        if (err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0) abort();
    }


    class Imgui {
    public:
        Imgui(Window &window, Device &device, VkRenderPass renderPass, uint32_t imageCount);
        ~Imgui();

        void newFrame();

        void render(VkCommandBuffer commandBuffer);

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        void runExample();

        void drawImage();

        VkDescriptorSet addTexture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
        void removeTexture(VkDescriptorSet id);

    private:
        Device &mDevice;

        // We haven't yet covered descriptor pools in the tutorial series,
        // so I'm just going to create one for just imgui and store it here for now.
        // maybe its preferred to have a separate descriptor pool for imgui anyway,
        // I haven't looked into imgui best practices at all.
        VkDescriptorPool descriptorPool;
    };
}