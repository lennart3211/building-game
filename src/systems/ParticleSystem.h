#pragma once

#include "Particle.h"
#include "ParticleRenderSystem.h"
#include "BoxRenderSystem.h"

namespace engine {

    class ParticleSystem {
    private:
//        std::vector<Particle> m_particles;
//        std::vector<PhysicsParticle> m_physicsParticles;
        ParticleRenderSystem m_renderSystem;

//        std::vector<Box> m_boxes;
        BoxRenderSystem m_boxRenderSystem;

    public:
        ParticleSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, uint32_t maxParticles, uint32_t maxBoxes)
            : m_renderSystem(device, renderPass, globalSetLayout, maxParticles), m_boxRenderSystem(device, renderPass, globalSetLayout, maxBoxes) {}

//        void AddParticle(const glm::vec3 &position, const glm::vec4 &color, float radius, const glm::vec3 &velocity,
//                         float mass, bool isStatic);

//        void AddBox(const glm::vec3 &position, const glm::vec3 &halfExtent);

        void Update(FrameInfo &frameInfo);
        void Render(FrameInfo &frameInfo);

//        [[nodiscard]] std::pair<std::vector<Particle> &, std::vector<PhysicsParticle> &> Particles();
//        [[nodiscard]] std::vector<Box> &Boxes() { return m_boxes; }

    private:
//        void RemoveParticle(uint32_t i);

      void CheckCollisions(entt::registry &registry);
        void CheckBoxCollisions(entt::registry &registry);
        void HandleCollision(entt::entity e1, entt::entity e2,
                             entt::registry &registry);
    };

} // engine
