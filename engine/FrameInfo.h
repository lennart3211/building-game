#pragma once


#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace engine {

    struct GlobalUbo {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::vec4 ambientLightColor{0.3f, 0.3f, 1.0f, .02f};
        glm::vec3 lightPosition{1, -1, 0};
        glm::vec4 lightColor{1};
    };

    struct FrameInfo {
        int frameIndex;
        float dt;
        VkCommandBuffer commandBuffer;
        std::vector<VkDescriptorSet> descriptorSets;
        entt::registry &registry;
    };
}

