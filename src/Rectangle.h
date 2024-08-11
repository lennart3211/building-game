#pragma once

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <experimental/vector>
#include <vulkan/vulkan.h>

namespace engine {

struct Rectangle {
  alignas(8)  glm::vec2 position;
  alignas(8)  glm::vec2 halfExtent;
  alignas(16) glm::vec4 color;

  Rectangle(const glm::vec2 &position, const glm::vec2 &extent, const glm::vec4 &color)
      : position(position), halfExtent(extent), color(color) {}

  static std::vector<VkVertexInputBindingDescription> getBindingDescription();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

} // namespace engine
