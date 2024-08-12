//
// Created by mclen on 01/08/2024.
//

#ifndef PARTICLES_COMPONENT_H
#define PARTICLES_COMPONENT_H

#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "glm/vec3.hpp"

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
      glm::vec3 translation{};
      glm::vec3 scale{1.0f, 1.0f, 1.0f};
      glm::vec3 rotation;

      [[nodiscard]] glm::mat4 mat4() const;
      [[nodiscard]] glm::mat3 normalMatrix() const;
    };

    struct entity_info {
        std::string name;
    };

} // namespace engine::component

#endif // PARTICLES_COMPONENT_H
