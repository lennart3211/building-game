#pragma once

#include "Device.h"
#include "Model.h"

namespace engine {

    class RayTracingModel {
    public:
        RayTracingModel(Device &device, Model::Builder &builder);

        [[nodiscard]] VkAccelerationStructureKHR blas() const { return m_blas; }

    private:
        void CreateVertexBuffers(const std::vector<Model::Vertex> &vertices);

        void CreateIndexBuffer(const std::vector<uint32_t> &indices);

        void CreateAccelerationStructure();

    private:
        Device &m_device;
        uint32_t m_vertexCount, m_indexCount;
        std::unique_ptr<Buffer> m_vertexBuffer, m_indexBuffer;
        std::unique_ptr<Buffer> m_blasBuffer;
        VkAccelerationStructureKHR m_blas;
        bool m_hasIndexBuffer;
    };

} // engine
