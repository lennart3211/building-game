//
// Created by lennart on 6/7/24.
//

#include "RayTracingModel.h"

namespace engine {
    RayTracingModel::RayTracingModel(Device &device, Model::Builder &builder) : m_device(device) {
            CreateVertexBuffers(builder.vertices);
            CreateIndexBuffer(builder.indices);
            CreateAccelerationStructure();
    }

    void RayTracingModel::CreateVertexBuffers(const std::vector<Model::Vertex> &vertices) {
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        Buffer stagingBuffer{
                m_device,
                vertexSize,
                m_vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) vertices.data());

        m_vertexBuffer = std::make_unique<Buffer>(m_device, vertexSize, m_vertexCount,
                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
    }

    void RayTracingModel::CreateIndexBuffer(const std::vector<uint32_t> &indices) {
        m_indexCount = static_cast<uint32_t>(indices.size());
        m_hasIndexBuffer = m_indexCount > 0;

        if (!m_hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
                m_device,
                indexSize,
                m_indexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) indices.data());

        m_indexBuffer = std::make_unique<Buffer>(
                m_device,
                indexSize,
                m_indexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
    }

    void RayTracingModel::CreateAccelerationStructure() {
        VkCommandBuffer cmdBuffer = m_device.beginSingleTimeCommands();

        VkDeviceAddress vertexBufferAddress = m_vertexBuffer->getBufferDeviceAddress();
        VkDeviceAddress indexBufferAddress  = m_indexBuffer->getBufferDeviceAddress();

        VkAccelerationStructureGeometryTrianglesDataKHR triangles{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR};
        triangles.vertexFormat             = VK_FORMAT_R32G32B32_SFLOAT;
        triangles.vertexData.deviceAddress = vertexBufferAddress;
        triangles.vertexStride             = 3 * sizeof(float);
        triangles.indexType                = VK_INDEX_TYPE_UINT32;
        triangles.indexData.deviceAddress  = indexBufferAddress;
        triangles.maxVertex                = uint32_t(m_vertexCount - 1);
        triangles.transformData            = {0};

        VkAccelerationStructureGeometryKHR geometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
        geometry.geometryType       = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometry.geometry.triangles = triangles;
        geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

        VkAccelerationStructureBuildRangeInfoKHR rangeInfo;
        rangeInfo.firstVertex     = 0;
        rangeInfo.primitiveCount  = uint32_t(m_indexCount / 3);
        rangeInfo.primitiveOffset = 0;
        rangeInfo.transformOffset = 0;

        VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
        buildInfo.flags                    = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        buildInfo.geometryCount            = 1;
        buildInfo.pGeometries              = &geometry;
        buildInfo.mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

        VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};

        auto vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_device.device(), "vkGetAccelerationStructureBuildSizesKHR"));
        vkGetAccelerationStructureBuildSizesKHR(
                m_device.device(),
                VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                &buildInfo,
                &rangeInfo.primitiveCount,
                &sizeInfo);

        m_blasBuffer = std::make_unique<Buffer>(m_device,
                                                sizeInfo.accelerationStructureSize,
                                                1,
                                                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
                                                | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                                                | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // idk if this flag is right
        );

        VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
        createInfo.type   = buildInfo.type;
        createInfo.size   = sizeInfo.accelerationStructureSize;
        createInfo.buffer = m_blasBuffer->getBuffer();
        createInfo.offset = 0;
        auto vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_device.device(), "vkCreateAccelerationStructureKHR"));
        vkCreateAccelerationStructureKHR(m_device.device(), &createInfo, nullptr, &m_blas);
        buildInfo.dstAccelerationStructure = m_blas;

        Buffer scratchBuffer{m_device,
                             sizeInfo.buildScratchSize,
                             1,
                             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                             | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        buildInfo.scratchData.deviceAddress = scratchBuffer.getBufferDeviceAddress();

        VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;
        auto vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_device.device(), "vkCmdBuildAccelerationStructuresKHR"));
        vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildInfo, &pRangeInfo);
        m_device.endSingleTimeCommands(cmdBuffer);
    }
} // engine