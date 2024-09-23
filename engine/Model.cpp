#include "Model.h"
#include "Error.h"

#include <vulkan/vulkan_core.h>

#define TINYOBJLOADER_IMPLEMENTATION

#include "Component.h"
#include "tiny_obj_loader/tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>

namespace std {
    template<>
    struct hash<engine::Model::Vertex> {
        size_t operator()(engine::Model::Vertex const &vertex) const {
            size_t seed = 0;
            engine::HashCombine(seed, vertex.position, vertex.uv);
            return seed;
        }
    };
}

namespace engine {
    Model::Model(Device &device, const Builder &builder, uint32_t maxInstances)
            : m_device(device), m_maxInstances(maxInstances) {
        CreateVertexBuffers(builder.vertices);
        CreateIndexBuffer(builder.indices);
        FindMinMaxExtent(builder.vertices);
    }

    void Model::CreateVertexBuffers(const std::vector<Vertex> &vertices) {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        Buffer stagingBuffer{m_device,
                vertexSize,
                m_VertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) vertices.data());

        m_VertexBuffer = std::make_unique<Buffer>(m_device, vertexSize, m_VertexCount,
                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);
    }

    void Model::CreateIndexBuffer(const std::vector<uint32_t> &indices) {
        m_IndexCount = static_cast<uint32_t>(indices.size());
        m_HasIndexBuffer = m_IndexCount > 0;

        if (!m_HasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
            m_device,
                indexSize,
                m_IndexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) indices.data());

        m_IndexBuffer = std::make_unique<Buffer>(
            m_device,
                indexSize,
                m_IndexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);
    }

    void Model::Draw(VkCommandBuffer commandBuffer) const {
        if (m_HasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
        }
    }

    void Model::Bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {m_VertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_HasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::unique_ptr<Model> Model::CreateModelFromFile(Device &device, const std::string &filepath) {
        Builder builder{};
        builder.LoadModel(filepath);
        std::cout << "Vertex count: " << builder.vertices.size() << std::endl;
        return std::make_unique<Model>(device, builder);
    }

    glm::vec3 Model::GetMinExtents() const {
        return mMinExtent;
    }

    glm::vec3 Model::GetMaxExtents() const {
        return mMaxExtent;
    }

    void Model::FindMinMaxExtent(const std::vector<Vertex>& vertices) {
        mMinExtent = vertices[0].position;
        mMaxExtent = vertices[0].position;

        for (uint32_t i = 1; i < vertices.size(); i++) {
            mMinExtent.x = std::min(mMinExtent.x, vertices[i].position.x);
            mMinExtent.y = std::min(mMinExtent.y, vertices[i].position.y);
            mMinExtent.z = std::min(mMinExtent.z, vertices[i].position.z);

            mMaxExtent.x = std::max(mMaxExtent.x, vertices[i].position.x);
            mMaxExtent.y = std::max(mMaxExtent.y, vertices[i].position.y);
            mMaxExtent.z = std::max(mMaxExtent.z, vertices[i].position.z);
        }
    }

    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingsDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription>
    Model::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, (uint32_t) offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, (uint32_t) offsetof(Vertex, normal)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT, (uint32_t) offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

    bool Model::Builder::LoadModel(const std::string &filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        std::string mtlDir = filepath.substr(0, filepath.find_last_of('/') + 1);
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str(), mtlDir.c_str())) {
//            ERROR(err);
            return false;
        }

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint64_t> uniqueVertices{};
        for (const auto &shape: shapes) {
            size_t indexOffset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];

                // Get the material ID for the current face
                int materialId = shape.mesh.material_ids[f];

                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t index = shape.mesh.indices[indexOffset + v];

                    Vertex vertex{};

                    if (index.vertex_index >= 0) {
                        vertex.position = {
                                attrib.vertices[3 * index.vertex_index + 0],
                                attrib.vertices[3 * index.vertex_index + 1],
                                attrib.vertices[3 * index.vertex_index + 2]
                        };
                    }

                    if (index.normal_index >= 0) {
                        vertex.normal = {
                                attrib.normals[3 * index.normal_index + 0],
                                attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2]
                        };
                    }

                    if (index.texcoord_index >= 0) {
                        vertex.uv = {
                                attrib.texcoords[2 * index.texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                        };
                    }

                    // Use the material ID to access the corresponding material
                    if (materialId >= 0 && materialId < materials.size()) {
                        const tinyobj::material_t &material = materials[materialId];
                        // Access material properties, texture maps, etc.
                        // For example, to get the diffuse texture map:
                        std::string diffuseTexName = material.diffuse_texname;
                        // Load and assign the diffuse texture index to the vertex
                    }

                    if (uniqueVertices.count(vertex) == 0) {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[vertex]);
                }
                indexOffset += fv;
            }
        }

        return true;
    }
} // namespace engine
