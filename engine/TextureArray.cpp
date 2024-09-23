#include "TextureArray.h"
#include "Buffer.h"
#include "descriptors/DescriptorWriter.h"
#include "stb/stb_image.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace engine {

TextureArray::TextureArray(Device& device, const std::vector<std::string>& filepaths)
    : m_device(device), m_layerCount(static_cast<uint32_t>(filepaths.size())) {
  loadImages(filepaths);
  createImage();
  createImageView();
  createSampler();
  transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//  m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

TextureArray::~TextureArray() {
  vkDestroySampler(m_device.device(), m_sampler, nullptr);
  vkDestroyImageView(m_device.device(), m_imageView, nullptr);
  vkDestroyImage(m_device.device(), m_image, nullptr);
  vkFreeMemory(m_device.device(), m_memory, nullptr);
}

void TextureArray::loadImages(const std::vector<std::string>& filepaths) {
  m_maxExtent = {0, 0};
  VkDeviceSize totalSize = 0;

  for (const auto& filepath : filepaths) {
    int width, height, channels;
    stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
      throw std::runtime_error("Failed to load texture image: " + filepath);
    }

    VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    m_maxExtent.width = std::max(m_maxExtent.width, extent.width);
    m_maxExtent.height = std::max(m_maxExtent.height, extent.height);

    VkDeviceSize imageSize = extent.width * extent.height * 4;
    m_textureInfos.push_back({filepath, extent, totalSize});
    totalSize += imageSize;

    stbi_image_free(pixels);
  }

  m_format = VK_FORMAT_R8G8B8A8_SRGB;
  m_usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

  Buffer stagingBuffer{
      m_device,
      4,
      static_cast<uint32_t>(totalSize),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  };

  stagingBuffer.map();
  for (const auto& info : m_textureInfos) {
    int width, height, channels;
    stbi_uc* pixels = stbi_load(info.filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize imageSize = info.extent.width * info.extent.height * 4;
    stagingBuffer.writeToBuffer(pixels, imageSize, info.offset);
    stbi_image_free(pixels);
  }

  createImage();

  transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  copyBufferToImage(stagingBuffer.getBuffer());

  transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void TextureArray::createImage() {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = m_maxExtent.width;
  imageInfo.extent.height = m_maxExtent.height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = m_layerCount;
  imageInfo.format = m_format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = m_usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_memory);
}

void TextureArray::createImageView() {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = m_image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  viewInfo.format = m_format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = m_layerCount;

  if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }
}

void TextureArray::createSampler() {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = m_device.properties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
}

void TextureArray::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
  auto commandBuffer = m_device.beginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = m_image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = m_layerCount;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage, destinationStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
  );

  m_device.endSingleTimeCommands(commandBuffer);
  m_layout = newLayout;
  std::cout << "Transitioning image layout from " << oldLayout << " to " << newLayout << std::endl;
}

void TextureArray::copyBufferToImage(VkBuffer buffer) {
  VkCommandBuffer commandBuffer = m_device.beginSingleTimeCommands();

  std::vector<VkBufferImageCopy> regions;
  for (uint32_t i = 0; i < m_layerCount; i++) {
    const auto& info = m_textureInfos[i];
    VkBufferImageCopy region{};
    region.bufferOffset = info.offset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = i;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {info.extent.width, info.extent.height, 1};
    regions.push_back(region);
  }

  vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         static_cast<uint32_t>(regions.size()), regions.data());

  m_device.endSingleTimeCommands(commandBuffer);
}

void TextureArray::writeDescriptorSets(DescriptorPool& descriptorPool, DescriptorSetLayout& descriptorSetLayout) {
  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = m_layout;
  imageInfo.imageView = m_imageView;
  imageInfo.sampler = m_sampler;

  DescriptorWriter{descriptorSetLayout, descriptorPool}
      .writeImage(0, &imageInfo)
      .build(m_descriptorSet);
}
void TextureArray::transitionLayout(VkImageLayout newLayout) {
  if (m_layout != newLayout) {
    transitionImageLayout(m_layout, newLayout);
  }
}

} // namespace engine