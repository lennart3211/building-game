#pragma once

#include "Buffer.h"
#include "Device.h"
#include "FrameInfo.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Rectangle.h"

#include <memory>

namespace engine {

class MeshRenderSystem {
private:
  Device &m_device;
  std::unique_ptr<Pipeline> m_pipeline;
  VkPipelineLayout m_pipelineLayout;

public:
  MeshRenderSystem(Device &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> &&descriptorSetLayouts, const std::string &vertPath, const std::string &fragPath);

  ~MeshRenderSystem();

  MeshRenderSystem(const MeshRenderSystem &) = delete;

  MeshRenderSystem &operator=(const MeshRenderSystem &) = delete;

  void Render(FrameInfo &frameInfo);

private:
  void CreatePipelineLayout(std::vector<VkDescriptorSetLayout> &globalSetLayouts);
  void CreatePipeline(VkRenderPass renderPass, const std::string &vertPath,
                      const std::string &fragPath);
};

} // engine
