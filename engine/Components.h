#pragma once

#include "Model.h"

#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace engine::component {
    struct Transform {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation;

        [[nodiscard]] glm::mat4 mat4() const;

        [[nodiscard]] glm::mat3 normalMatrix() const;
    };

    struct PointLight {
        float lightIntensity = 1.0f;
    };

    struct RigidBody {
        glm::vec3 acceleration{0.0f};
        glm::vec3 velocity{0.0f};
        glm::vec3 netForce{0.0f};
        float inverseMass = 1.0f;
        bool isStatic = true;
    };

    struct Hitbox {
        std::unique_ptr<Buffer> vertexBuffer;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;
        glm::vec3 color;
    };

    struct AABBCollider {
        glm::vec3 minExtent;
        glm::vec3 maxExtent;

        bool checkCollision(const AABBCollider &other) const;
    };

    struct Color {
        glm::vec3 color;
    };

    struct Material {
        uint32_t textureIndex;
    };
}
