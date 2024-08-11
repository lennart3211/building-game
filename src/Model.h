#pragma once

#include "Buffer.h"
#include "Component.h"
#include "Device.h"
#include "Utils.h"

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

class transform;
namespace engine {
    class Model {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription>
            getBindingsDescriptions();

            static std::vector<VkVertexInputAttributeDescription>
            getAttributeDescriptions();

            bool operator==(const Vertex &other) const {
                return position == other.position && normal == other.normal && uv == other.uv;
            }
        };

        struct Instance {
            component::transform transform;
            VkDescriptorSet texture;
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void LoadModel(const std::string &filepath);
        };

        Model(Device &device, const Builder &builder, uint32_t maxInstances = 2);

        ~Model() = default;

        Model(const Model &) = delete;

        Model &operator=(const Model &) = delete;

        static std::unique_ptr<Model> CreateModelFromFile(Device &device, const std::string &filepath);

        void SetInstances(std::vector<component::transform> &instances);

        void Bind(VkCommandBuffer commandBuffer);

        void Draw(VkCommandBuffer commandBuffer) const;

        glm::vec3 GetMinExtents() const;
        glm::vec3 GetMaxExtents() const;


    private:
        void CreateVertexBuffers(const std::vector<Vertex> &vertices);

        void CreateIndexBuffer(const std::vector<uint32_t> &indices);

        void CreateInstanceBuffer(const std::vector<component::transform> &instances);
        void UpdateInstanceBuffer(const std::vector<component::transform> &instances);

        void FindMinMaxExtent(const std::vector<Vertex> &vertices);

        Device &m_device;

        std::unique_ptr<Buffer> m_VertexBuffer;
        uint32_t m_VertexCount;

        bool m_HasIndexBuffer{false};
        std::unique_ptr<Buffer> m_IndexBuffer;
        uint32_t m_IndexCount;

        std::unique_ptr<Buffer> m_instanceBuffer;
        uint32_t m_instanceCount;
        uint32_t m_maxInstances;

        glm::vec3 mMinExtent, mMaxExtent;
    };
} // namespace engine
