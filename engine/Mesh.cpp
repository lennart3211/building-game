//
// Created by mclen on 21/07/2024.
//

#include "Mesh.h"

namespace engine {
std::vector<VkVertexInputBindingDescription>
Mesh::Vertex::getBindingsDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
Mesh::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32_SFLOAT, (uint32_t) offsetof(Vertex, position)});
  attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32_SFLOAT, (uint32_t) offsetof(Vertex, uv)});
  attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, (uint32_t) offsetof(Vertex, color)});

  return attributeDescriptions;
}

Mesh::Mesh(Device &device, std::vector<Vertex> &vertices,
           std::vector<uint32_t> &indices)
    : m_device(device) {
  CreateVertexBuffer(vertices);
  CreateIndexBuffer(indices);
}

void Mesh::CreateVertexBuffer(std::vector<Vertex> &vertices) {
  m_vertexCount = static_cast<uint32_t>(vertices.size());
  assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  Buffer stagingBuffer{
      m_device,
      vertexSize,
      m_vertexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *) vertices.data());

  m_vertexBuffer = std::make_unique<Buffer>(m_device, vertexSize, m_vertexCount,
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  m_device.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
}

void Mesh::CreateIndexBuffer(std::vector<uint32_t> &indices) {
  m_indexCount = static_cast<uint32_t>(indices.size());

  VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
  uint32_t indexSize = sizeof(indices[0]);

  Buffer stagingBuffer{
      m_device,
      indexSize,
      m_indexCount,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *) indices.data());

  m_indexBuffer = std::make_unique<Buffer>(
      m_device,
      indexSize,
      m_indexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  m_device.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
}

void Mesh::Bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};

  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::Draw(VkCommandBuffer commandBuffer) const {
  vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
}
} // namespace engine