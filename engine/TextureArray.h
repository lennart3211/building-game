#pragma once

#include "Device.h"
#include "descriptors/DescriptorPool.h"
#include "descriptors/DescriptorSetLayout.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace engine {

struct TextureInfo {
  std::string filepath;
  VkExtent2D extent;
  VkDeviceSize offset;
};

class TextureArray {
private:
  Device& m_device;
  VkImage m_image{VK_NULL_HANDLE};
  VkDeviceMemory m_memory{VK_NULL_HANDLE};
  VkImageView m_imageView{VK_NULL_HANDLE};
  VkSampler m_sampler{VK_NULL_HANDLE};
  VkImageLayout m_layout;

  VkExtent2D m_maxExtent;
  VkFormat m_format;
  VkImageUsageFlags m_usage;
  uint32_t m_layerCount;
  uint32_t m_mipLevels{1};

  VkDescriptorSet m_descriptorSet{VK_NULL_HANDLE};
  std::vector<TextureInfo> m_textureInfos;

public:
  TextureArray(Device& device, const std::vector<std::string>& filepaths);
  ~TextureArray();

  TextureArray(const TextureArray&) = delete;
  TextureArray& operator=(const TextureArray&) = delete;

  [[nodiscard]] VkImage image() const { return m_image; }
  [[nodiscard]] VkImageView imageView() const { return m_imageView; }
  [[nodiscard]] VkSampler sampler() const { return m_sampler; }
  [[nodiscard]] VkExtent2D maxExtent() const { return m_maxExtent; }
  [[nodiscard]] VkFormat format() const { return m_format; }
  [[nodiscard]] uint32_t layerCount() const { return m_layerCount; }
  [[nodiscard]] const std::vector<TextureInfo>& textureInfos() const { return m_textureInfos; }
  [[nodiscard]] VkDescriptorSet descriptorSet() const { return m_descriptorSet; }

  void writeDescriptorSets(DescriptorPool& descriptorPool, DescriptorSetLayout& descriptorSetLayout);
  void transitionLayout(VkImageLayout newLayout);

private:
  void loadImages(const std::vector<std::string>& filepaths);
  void createImage();
  void createImageView();
  void createSampler();
  void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
  void copyBufferToImage(VkBuffer buffer);
};

} // namespace engine