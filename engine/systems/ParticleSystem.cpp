//
// Created by lennart on 7/26/24.
//

#include "ParticleSystem.h"
#include <iostream>

namespace engine {
//    void ParticleSystem::AddParticle(const glm::vec3 &position, const glm::vec4 &color, float radius,
//                                     const glm::vec3 &velocity,
//                                     float mass, bool isStatic) {
//        m_particles.emplace_back(position, color, radius);
//        m_physicsParticles.emplace_back(m_particles.size() - 1, velocity, mass, isStatic);
//        m_renderSystem.UpdateInstanceBuffer(m_particles);
//    }

//    void ParticleSystem::RemoveParticle(uint32_t i) {
//        if (i >= m_particles.size()) return;
//
//        assert(m_particles.size() == m_physicsParticles.size() && "m_particles and m_physicsParticles need to be the same size");
//
//        std::swap(m_particles[i], m_particles.back());
//        std::swap(m_physicsParticles[i], m_physicsParticles.back());
//
//        if (i < m_particles.size() - 1) {
//            m_physicsParticles[i].particleIndex = i;
//        }
//
//        m_particles.pop_back();
//        m_physicsParticles.pop_back();
//
////        m_renderSystem.UpdateInstanceBuffer(m_particles);
//    }

    void ParticleSystem::Render(FrameInfo &frameInfo) {
        m_renderSystem.Render(frameInfo);
        m_boxRenderSystem.Render(frameInfo);
    }

    void ParticleSystem::Update(FrameInfo &frameInfo) {
        CheckCollisions(frameInfo.registry);
        CheckBoxCollisions(frameInfo.registry);

        frameInfo.registry.view<Particle, component::physics>().each([&](auto &particle, auto &physics) {
            if (!physics.isStatic) physics.velocity += glm::vec3(0, 9.81, 0) * frameInfo.dt;
            particle.position += physics.velocity * frameInfo.dt;
        });

//        m_renderSystem.UpdateInstanceBuffer(m_particles);
//        m_boxRenderSystem.UpdateVertexBuffer(m_boxes);
    }

    void ParticleSystem::CheckCollisions(entt::registry &registry) {
        auto view = registry.view<Particle, component::physics>();
        view.each([&](auto e1, auto &particle1, auto &physics1) {
            view.each([&](auto e2, auto &particle2, auto &physics2) {
                if (e1 != e2) {
                    float dist = glm::length(particle1.position - particle2.position);
                    float minDist = particle1.radius + particle2.radius;

                    if (dist < minDist) {
                      HandleCollision(e1, e2, registry);
                    }
                }
            });
        });
    }

    void ParticleSystem::HandleCollision(entt::entity e1, entt::entity e2,
                                         entt::registry &registry) {
        auto &particle1 = registry.get<Particle>(e1);
        auto &particle2 = registry.get<Particle>(e2);
        auto &physics1 = registry.get<component::physics>(e1);
        auto &physics2 = registry.get<component::physics>(e2);

        glm::vec3 normal = glm::normalize(particle2.position - particle1.position);
        glm::vec3 rv = physics2.velocity - physics1.velocity;
        float velAlongNormal = glm::dot(rv, normal);

        if (velAlongNormal > 0)
            return;

        float e = 0.8f;

        float j = -(1 + e) * velAlongNormal;
        j /= 1 / physics1.mass + 1 / physics2.mass;

        // Apply impulse
        glm::vec3 impulse = j * normal;
        physics1.velocity -= 1 / physics1.mass * impulse;
        physics2.velocity += 1 / physics2.mass * impulse;
    }


//    void ParticleSystem::AddBox(const glm::vec3 &position, const glm::vec3 &halfExtent) {
//        m_boxes.emplace_back(position, halfExtent);
//        m_boxRenderSystem.UpdateVertexBuffer(m_boxes);
//    }

    void ParticleSystem::CheckBoxCollisions(entt::registry &registry) {
        registry.view<Particle, component::physics>().each([&](auto &particle, auto &physics) {
          registry.view<Box>().each([&](auto &box) {
            glm::vec3 relativePos = particle.position - box.position;
            glm::vec3 penetration;
            bool collision = false;

            for (uint8_t i = 0; i < 3; ++i) {
              if (std::abs(relativePos[i]) > box.halfExtent[i] - particle.radius) {
                float sign = glm::sign(relativePos[i]);
                penetration[i] = (box.halfExtent[i] - particle.radius) * sign - relativePos[i];
                collision = true;
              } else {
                penetration[i] = 0;
              }
            }

            if (collision) {
              int hitAxis = 0;
              for (int i = 1; i < 3; ++i) {
                if (std::abs(penetration[i]) > std::abs(penetration[hitAxis])) {
                  hitAxis = i;
                }
              }

              particle.position[hitAxis] += penetration[hitAxis];

              physics.velocity[hitAxis] *= -0.8f;
            }
          });
        });
    }
} // engine