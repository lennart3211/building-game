#pragma once

#include "Device.h"
#include "Pipeline.h"
#include "FrameInfo.h"
#include "Buffer.h"
#include "Particle.h"

#include <memory>

namespace engine {

class BoxRenderSystem {
private:
  Device &m_device;
  std::unique_ptr<Pipeline> m_pipeline;
  VkPipelineLayout m_pipelineLayout;

  const uint32_t m_maxBoxes;

  std::unique_ptr<Buffer> m_vertexBuffer;

public:
  BoxRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, uint32_t maxBoxes);

  ~BoxRenderSystem();

  BoxRenderSystem(const BoxRenderSystem &) = delete;

  BoxRenderSystem &operator=(const BoxRenderSystem &) = delete;

  void Render(FrameInfo &frameInfo);

  void CreateVertexBuffer(const std::vector<Box> &boxes);
  void UpdateVertexBuffer(const std::vector<Box> &boxes);

private:
  void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void CreatePipeline(VkRenderPass renderPass);

  void Bind(VkCommandBuffer commandBuffer);
};

} // engine
