#pragma ocne

#include "Device.h"

#include <vulkan/vulkan.h>

namespace engine {

    class Texture {
        VkImage m_image{VK_NULL_HANDLE};
        VkDeviceMemory m_memory{VK_NULL_HANDLE};
        VkImageView m_imageView{VK_NULL_HANDLE};
        VkSampler m_sampler{VK_NULL_HANDLE};

        VkExtent2D m_extent;
        VkFormat m_format;
        VkImageUsageFlags m_usage;
        VkImageAspectFlags m_aspect;

        Device &m_device;

    public:
        Texture(Device &device, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        [[nodiscard]] VkImage image() const { return m_image; }
        [[nodiscard]] VkImageView imageView() const { return m_imageView; }
        [[nodiscard]] VkSampler sampler() const { return m_sampler; }
        [[nodiscard]] VkExtent2D extent() const { return m_extent; }
        [[nodiscard]] VkFormat format() const { return m_format; }

        void transitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyFromBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer);
        void copyFromImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkExtent2D srcExtent);

    private:
        void createImage();
        void createImageView();
        void createSampler();

    };

} // engine
