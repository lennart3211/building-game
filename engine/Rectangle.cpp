//
// Created by mclen on 20/07/2024.
//

#include "Rectangle.h"

namespace engine {
std::vector<VkVertexInputBindingDescription> Rectangle::getBindingDescription() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Rectangle);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Rectangle::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

  attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32_SFLOAT, (uint32_t) offsetof(Rectangle, position)});
  attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32_SFLOAT, (uint32_t) offsetof(Rectangle, halfExtent)});
  attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, (uint32_t) offsetof(Rectangle, color)});

  return attributeDescriptions;
}
} // namespace engine