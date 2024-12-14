//
// Created by mclen on 01/08/2024.
//

#ifndef PARTICLES_COMPONENT_H
#define PARTICLES_COMPONENT_H

#include "entt/entity/entity.hpp"
#include "glm/vec3.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <string>
#include <vulkan/vulkan_core.h>

namespace engine::component {
    struct physics {
      glm::vec3 velocity;
      float mass;
      bool isStatic;
    };

    struct box_collider {
      glm::vec3 halfExtent;
    };

    struct sphere_collider {
      float radius;
    };

    struct transform {
      glm::vec3 translation{0};
      glm::vec3 scale{1.0f, 1.0f, 1.0f};
      glm::vec3 rotation{0};

      [[nodiscard]] glm::mat4 mat4() const;
      [[nodiscard]] glm::mat3 normalMatrix() const;
    };

    struct entity_info {
        entt::entity entity;
        std::string name;
    };

    struct texture_index {
        uint32_t index{0};
    };

    struct material {
        VkDescriptorSet descriptorSet;
    };

} // namespace engine::component

#endif // PARTICLES_COMPONENT_H
