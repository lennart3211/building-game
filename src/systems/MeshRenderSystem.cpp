#include "MeshRenderSystem.h"
#include "Model.h"
#include <stdexcept>

namespace engine {

struct SimplePushConstantsData {
  glm::mat3 modelMatrix{1.0f};
};

MeshRenderSystem::MeshRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, const std::string &vertPath, const std::string &fragPath)
    : m_device(device) {
  CreatePipelineLayout(globalSetLayout);
  CreatePipeline(renderPass, vertPath, fragPath);
}

MeshRenderSystem::~MeshRenderSystem() {
  vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
}

void MeshRenderSystem::Render(FrameInfo &frameInfo) {
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

  frameInfo.registry.view<Model, Model::Instance>().each([&](auto &model, auto &instance) { // may need to be ptr to model
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0,1,
        &instance.texture,
        0,nullptr
    );
    model.Bind(frameInfo.commandBuffer);
    model.Draw(frameInfo.commandBuffer);
  });
}

void MeshRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout) {
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

void MeshRenderSystem::CreatePipeline(VkRenderPass renderPass,
                                      const std::string &vertPath,
                                      const std::string &fragPath) {
  assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  pipelineConfig.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  // Changed to triangle list
  pipelineConfig.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  pipelineConfig.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  pipelineConfig.viewportInfo.viewportCount = 1;
  pipelineConfig.viewportInfo.pViewports = nullptr;
  pipelineConfig.viewportInfo.scissorCount = 1;
  pipelineConfig.viewportInfo.pScissors = nullptr;

  pipelineConfig.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  pipelineConfig.rasterizationInfo.depthClampEnable = VK_FALSE;
  pipelineConfig.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
  pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
  pipelineConfig.rasterizationInfo.lineWidth = 1.0f;
  pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;  // Enable back-face culling
  pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;  // Assuming counter-clockwise winding
  pipelineConfig.rasterizationInfo.depthBiasEnable = VK_FALSE;
  pipelineConfig.rasterizationInfo.depthBiasConstantFactor = 0.0f;
  pipelineConfig.rasterizationInfo.depthBiasClamp = 0.0f;
  pipelineConfig.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

  pipelineConfig.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  pipelineConfig.multisampleInfo.sampleShadingEnable = VK_FALSE;
  pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  pipelineConfig.multisampleInfo.minSampleShading = 1.0f;
  pipelineConfig.multisampleInfo.pSampleMask = nullptr;
  pipelineConfig.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
  pipelineConfig.multisampleInfo.alphaToOneEnable = VK_FALSE;

  pipelineConfig.colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
  pipelineConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  pipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  pipelineConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  pipelineConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  pipelineConfig.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  pipelineConfig.colorBlendInfo.logicOpEnable = VK_FALSE;
  pipelineConfig.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
  pipelineConfig.colorBlendInfo.attachmentCount = 1;
  pipelineConfig.colorBlendInfo.pAttachments = &pipelineConfig.colorBlendAttachment;
  pipelineConfig.colorBlendInfo.blendConstants[0] = 0.0f;
  pipelineConfig.colorBlendInfo.blendConstants[1] = 0.0f;
  pipelineConfig.colorBlendInfo.blendConstants[2] = 0.0f;
  pipelineConfig.colorBlendInfo.blendConstants[3] = 0.0f;

  pipelineConfig.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;  // Enable depth testing
  pipelineConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;  // Enable depth writing
  pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
  pipelineConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  pipelineConfig.depthStencilInfo.minDepthBounds = 0.0f;
  pipelineConfig.depthStencilInfo.maxDepthBounds = 1.0f;
  pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
  pipelineConfig.depthStencilInfo.front = {};
  pipelineConfig.depthStencilInfo.back = {};

  pipelineConfig.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  pipelineConfig.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  pipelineConfig.dynamicStateInfo.pDynamicStates = pipelineConfig.dynamicStateEnables.data();
  pipelineConfig.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(pipelineConfig.dynamicStateEnables.size());
  pipelineConfig.dynamicStateInfo.flags = 0;

  pipelineConfig.bindingDescriptions = Mesh::Vertex::getBindingsDescriptions();
  pipelineConfig.attributeDescriptions = Mesh::Vertex::getAttributeDescriptions();
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;

  pipelineConfig.vertPath = vertPath;
  pipelineConfig.fragPath = fragPath;

  m_pipeline = std::make_unique<Pipeline>(m_device, pipelineConfig);
}
} // engine