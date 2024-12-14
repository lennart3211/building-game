#pragma once

#include "Device.h"
#include "Pipeline.h"
#include "FrameInfo.h"
#include "Model.h"

namespace engine {

class ShadowRenderSystem {
private:
  struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f};
  };

  static constexpr int SHADOW_MAP_SIZE = 2048;

  Device& m_device;
  std::unique_ptr<Pipeline> m_pipeline;
  VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};

  // Shadow map resources
  VkImage m_depthImage{VK_NULL_HANDLE};
  VkDeviceMemory m_depthImageMemory{VK_NULL_HANDLE};
  VkImageView m_depthImageView{VK_NULL_HANDLE};
  VkSampler m_sampler{VK_NULL_HANDLE};
  VkRenderPass m_renderPass{VK_NULL_HANDLE};
  VkFramebuffer m_shadowFramebuffer{VK_NULL_HANDLE};

  // Descriptor resources
  std::unique_ptr<DescriptorPool> m_descriptorPool;
  std::unique_ptr <DescriptorSetLayout> m_descriptorSetLayout;
  VkDescriptorSet m_shadowMapDescriptorSet{VK_NULL_HANDLE};

public:
  ShadowRenderSystem(Device& device, VkDescriptorSetLayout globalSetLayout);
  ~ShadowRenderSystem();

  ShadowRenderSystem(const ShadowRenderSystem&) = delete;
  ShadowRenderSystem& operator=(const ShadowRenderSystem&) = delete;

  void Render(FrameInfo& frameInfo);
  VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout->getDescriptorSetLayout(); }
  VkDescriptorSet GetShadowMapDescriptorSet();

private:
  void CreateDepthResources();
  void CreateSampler();
  void CreateRenderPass();
  void CreateFramebuffer();
  void CreateDescriptorSetLayout();
  void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void CreatePipeline();
  void CreateDescriptorPool();
  void CreateDescriptorSet();
};

} // namespace engine