//
// Created by lennart on 7/5/24.
//

#pragma once

#include "glm/vec3.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace engine {

    struct Particle {
        glm::vec3 position;
        glm::vec4 color;
        float radius;

        Particle()
                : position(glm::vec3(0)), radius(0), color(glm::vec4(1)) {}

        Particle(const glm::vec3 &position, const float size)
            : position(position), radius(size), color(glm::vec4(1)) {}

        Particle(const glm::vec3 &position, const glm::vec4 &color, const float size)
                : position(position), radius(size), color(color) {}

        static std::vector<VkVertexInputBindingDescription> getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    struct PhysicsParticle {
        size_t particleIndex;
        glm::vec3 velocity;
        float mass;
        bool isStatic;

        PhysicsParticle(size_t index, const glm::vec3& vel, float mass, bool isStatic)
                : particleIndex(index), velocity(vel), mass(mass), isStatic(isStatic) {}
    };

    struct Box {
        glm::vec3 position;
        glm::vec3 halfExtent;

        Box(const glm::vec3 &position, const glm::vec3 halfExtent) : position(position), halfExtent(halfExtent) {}

        static std::vector<VkVertexInputBindingDescription> getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };


} // engine
