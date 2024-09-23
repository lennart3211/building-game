#include "ParticleRenderSystem.h"

#include <stdexcept>

namespace engine {

    struct SimplePushConstantsData {
        glm::mat3 modelMatrix{1.0f};
    };

    ParticleRenderSystem::ParticleRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, uint32_t maxParticles)
    : m_device(device), m_maxParticles(maxParticles) {

        CreatePipelineLayout(globalSetLayout);
        CreatePipeline(renderPass);

        SetParticleMesh("../model/sphere.obj");

        CreateInstanceBuffer();
    }

    ParticleRenderSystem::~ParticleRenderSystem() {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void ParticleRenderSystem::Render(FrameInfo &frameInfo) {
        std::vector<Particle> particles;
        frameInfo.registry.view<Particle>().each([&](auto &particle) {
          particles.push_back(particle);
        });
        UpdateInstanceBuffer(particles);
        m_pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                0,
                frameInfo.descriptorSets.size(),
                frameInfo.descriptorSets.data(),
                0,
                nullptr
        );



        Bind(frameInfo.commandBuffer);
        vkCmdBindIndexBuffer(frameInfo.commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frameInfo.commandBuffer, m_indexCount, particles.size(), 0, 0, 0);
    }

    void ParticleRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantsData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void ParticleRenderSystem::CreatePipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        pipelineConfig.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineConfig.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        pipelineConfig.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineConfig.viewportInfo.viewportCount = 1;
        pipelineConfig.viewportInfo.pViewports = nullptr;
        pipelineConfig.viewportInfo.scissorCount = 1;
        pipelineConfig.viewportInfo.pScissors = nullptr;

        pipelineConfig.rasterizationInfo.sType =VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineConfig.rasterizationInfo.depthClampEnable = VK_FALSE;
        pipelineConfig.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        pipelineConfig.rasterizationInfo.lineWidth = 1.0f;
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipelineConfig.rasterizationInfo.depthBiasEnable = VK_FALSE;
        pipelineConfig.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
        pipelineConfig.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
        pipelineConfig.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

        pipelineConfig.multisampleInfo.sType =VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineConfig.multisampleInfo.sampleShadingEnable = VK_FALSE;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineConfig.multisampleInfo.minSampleShading = 1.0f;          // Optional
        pipelineConfig.multisampleInfo.pSampleMask = nullptr;            // Optional
        pipelineConfig.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        pipelineConfig.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

        pipelineConfig.colorBlendAttachment.colorWriteMask =VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
        pipelineConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipelineConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        pipelineConfig.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineConfig.colorBlendInfo.logicOpEnable = VK_FALSE;
        pipelineConfig.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        pipelineConfig.colorBlendInfo.attachmentCount = 1;
        pipelineConfig.colorBlendInfo.pAttachments = &pipelineConfig.colorBlendAttachment;
        pipelineConfig.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
        pipelineConfig.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
        pipelineConfig.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
        pipelineConfig.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

        pipelineConfig.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        pipelineConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.minDepthBounds = 0.0f; // Optional
        pipelineConfig.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
        pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.front = {}; // Optional
        pipelineConfig.depthStencilInfo.back = {};  // Optional

        pipelineConfig.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        pipelineConfig.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineConfig.dynamicStateInfo.pDynamicStates = pipelineConfig.dynamicStateEnables.data();
        pipelineConfig.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(pipelineConfig.dynamicStateEnables.size());
        pipelineConfig.dynamicStateInfo.flags = 0;

        pipelineConfig.bindingDescriptions = Particle::getBindingDescription();
        pipelineConfig.attributeDescriptions = Particle::getAttributeDescriptions();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        pipelineConfig.vertPath = "../shader/particle.vert.spv";
        pipelineConfig.fragPath = "../shader/particle.frag.spv";

        m_pipeline = std::make_unique<Pipeline>(m_device, pipelineConfig);
    }

    void ParticleRenderSystem::CreateVertexBuffer(
        const std::vector<Model::Vertex> &vertices) {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        assert(bufferSize > 0 && "Buffer size cannot be zero");

        Buffer stagingBuffer{m_device,
                             bufferSize,
                             1,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) vertices.data());

        m_vertexBuffer = std::make_unique<Buffer>(m_device,
                                                  sizeof(vertices[0]),
                                                  vertices.size(),
                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        m_device.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
        m_vertexCount = static_cast<uint32_t>(vertices.size());
    }

    void ParticleRenderSystem::CreateInstanceBuffer() {
        VkDeviceSize bufferSize = sizeof(Particle) * m_maxParticles;

        std::vector<Particle> dummy(m_maxParticles);

        assert(bufferSize > 0 && "Buffer size cannot be zero");

        Buffer stagingBuffer{m_device,
                             bufferSize,
                             1,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) dummy.data());

        m_instanceBuffer = std::make_unique<Buffer>(m_device,
                                                  sizeof(Particle),
                                                  m_maxParticles,
                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        m_device.copyBuffer(stagingBuffer.getBuffer(), m_instanceBuffer->getBuffer(), bufferSize);
    }

    void ParticleRenderSystem::CreateIndexBuffer(
        const std::vector<uint32_t> &indices) {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        Buffer stagingBuffer{m_device, bufferSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) indices.data());

        m_indexBuffer = std::make_unique<Buffer>(m_device, sizeof(indices[0]), indices.size(),
                                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
        m_indexCount = static_cast<uint32_t>(indices.size());
    }

    void ParticleRenderSystem::UpdateInstanceBuffer(const std::vector<Particle> &particles) {
        VkDeviceSize bufferSize = sizeof(particles[0]) * m_maxParticles;

        Buffer stagingBuffer{m_device, bufferSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) particles.data());

        m_device.copyBuffer(stagingBuffer.getBuffer(), m_instanceBuffer->getBuffer(), bufferSize);
    }



    void ParticleRenderSystem::Bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {m_vertexBuffer->getBuffer(), m_instanceBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0, 0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
    }

    void ParticleRenderSystem::SetParticleMesh(const std::string &filepath) {
        Model::Builder builder{};
        builder.LoadModel(filepath);
        CreateVertexBuffer(builder.vertices);
        CreateIndexBuffer(builder.indices);
    }

    } // engine