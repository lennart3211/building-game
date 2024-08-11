#pragma once

#include "Buffer.h"
#include "Device.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace engine {

struct Mesh {
  struct Vertex {
    glm::vec2 position;
    glm::vec2 uv;
    glm::vec4 color;

    static std::vector<VkVertexInputBindingDescription>
    getBindingsDescriptions();

    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return position == other.position && color == other.color;
    }
  };

public:
  Mesh(Device &device, std::vector<Vertex> &vertices,
       std::vector<uint32_t> &indices);



  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  void Bind(VkCommandBuffer commandBuffer);
  void Draw(VkCommandBuffer commandBuffer) const;

private:
  void CreateVertexBuffer(std::vector<Vertex> &vertices);
  void CreateIndexBuffer(std::vector<uint32_t> &indices);

private:
  Device &m_device;
  std::unique_ptr<Buffer> m_vertexBuffer;
  uint32_t m_vertexCount;
  std::unique_ptr<Buffer> m_indexBuffer;
  uint32_t m_indexCount;


};

} // namespace engine
