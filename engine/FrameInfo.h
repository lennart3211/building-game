#pragma once


#include <vulkan/vulkan.h>

#include "ResourceManager.h"
#include "Structure.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine {


    struct GlobalUbo {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::mat4 lightSpaceMatrix;
        glm::vec3 viewPosition;
        glm::vec4 ambientLightColor{0.3f, 0.3f, 1.0f, .02f};
        glm::vec3 lightPosition{-2, -4, -1};
        glm::vec4 lightColor{1};
    };

    struct FrameInfo {
        int frameIndex;
        float dt;
        VkCommandBuffer commandBuffer;
        std::vector<VkDescriptorSet> descriptorSets;
        const std::vector<std::optional<Structure>> &structures;
        ResourceManager &resourceManager;
    };
}

