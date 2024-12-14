//
// Created by mclen on 11/12/2024.
//

#include "ShadowRenderSystem.h"
#include <stdexcept>

namespace engine {

ShadowRenderSystem::ShadowRenderSystem(Device &device, VkDescriptorSetLayout globalSetLayout) : m_device(device) {
  CreateDepthResources();
  CreateSampler();
  CreateRenderPass();
  CreateFramebuffer();
  CreateDescriptorSetLayout();
  CreatePipelineLayout(globalSetLayout);
  CreatePipeline();
}

ShadowRenderSystem::~ShadowRenderSystem() {
  vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
  vkDestroySampler(m_device.device(), m_sampler, nullptr);
  vkDestroyImageView(m_device.device(), m_depthImageView, nullptr);
  vkDestroyImage(m_device.device(), m_depthImage, nullptr);
  vkFreeMemory(m_device.device(), m_depthImageMemory, nullptr);
  vkDestroyFramebuffer(m_device.device(), m_shadowFramebuffer, nullptr);
  vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);
}

void ShadowRenderSystem::CreateDepthResources() {
  VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = SHADOW_MAP_SIZE;
  imageInfo.extent.height = SHADOW_MAP_SIZE;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = depthFormat;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                               m_depthImage, m_depthImageMemory);

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = m_depthImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = depthFormat;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_depthImageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create depth image view!");
  }
}

void ShadowRenderSystem::CreateRenderPass() {
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = VK_FORMAT_D32_SFLOAT;
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkAttachmentReference depthReference{};
  depthReference.attachment = 0;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 0;
  subpass.pDepthStencilAttachment = &depthReference;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &depthAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  if (vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shadow render pass!");
  }
}

void ShadowRenderSystem::CreateFramebuffer() {
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = m_renderPass;
  framebufferInfo.attachmentCount = 1;
  framebufferInfo.pAttachments = &m_depthImageView;
  framebufferInfo.width = SHADOW_MAP_SIZE;
  framebufferInfo.height = SHADOW_MAP_SIZE;
  framebufferInfo.layers = 1;

  if (vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &m_shadowFramebuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shadow framebuffer!");
  }
}

void ShadowRenderSystem::CreateDescriptorSetLayout() {
  m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
                              .addBinding(
                                  0,
                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                  VK_SHADER_STAGE_FRAGMENT_BIT)
                              .build();
}

void ShadowRenderSystem::CreateSampler() {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  samplerInfo.compareEnable = VK_TRUE;
  samplerInfo.compareOp = VK_COMPARE_OP_LESS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 1.0f;

  if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create shadow map sampler!");
  }
}

void ShadowRenderSystem::Render(FrameInfo &frameInfo) {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_renderPass;
  renderPassInfo.framebuffer = m_shadowFramebuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};

  VkClearValue clearValue{};
  clearValue.depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearValue;

  vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(SHADOW_MAP_SIZE);
  viewport.height = static_cast<float>(SHADOW_MAP_SIZE);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(frameInfo.commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};
  vkCmdSetScissor(frameInfo.commandBuffer, 0, 1, &scissor);

  m_pipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(
      frameInfo.commandBuffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      m_pipelineLayout,
      0,
      static_cast<uint32_t>(frameInfo.descriptorSets.size()),
      frameInfo.descriptorSets.data(),
      0,
      nullptr
  );

  for (auto& obj : frameInfo.structures) {
    if (!obj) continue;
    SimplePushConstantData push{};
    push.modelMatrix = obj->mat4();

    vkCmdPushConstants(
        frameInfo.commandBuffer,
        m_pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(SimplePushConstantData),
        &push);

    auto model = frameInfo.resourceManager.getModel(obj->type);
    model->Bind(frameInfo.commandBuffer);
    model->Draw(frameInfo.commandBuffer);
  }

  vkCmdEndRenderPass(frameInfo.commandBuffer);
}

void ShadowRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shadow pipeline layout!");
  }
}

void ShadowRenderSystem::CreateDescriptorPool() {
  m_descriptorPool = DescriptorPool::Builder(m_device)
                         .setMaxSets(1)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
                         .build();
}

void ShadowRenderSystem::CreateDescriptorSet() {
  if (!m_descriptorPool->allocateDescriptor(
          m_descriptorSetLayout->getDescriptorSetLayout(),
          m_shadowMapDescriptorSet)) {
    throw std::runtime_error("Failed to allocate shadow map descriptor set");
  }

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = m_depthImageView;
  imageInfo.sampler = m_sampler;

  VkWriteDescriptorSet descriptorWrite{};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = m_shadowMapDescriptorSet;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(m_device.device(), 1, &descriptorWrite, 0, nullptr);
}

VkDescriptorSet ShadowRenderSystem::GetShadowMapDescriptorSet() {
  if (m_shadowMapDescriptorSet == VK_NULL_HANDLE) {
    if (!m_descriptorPool) {
      CreateDescriptorPool();
    }
    CreateDescriptorSet();
  }
  return m_shadowMapDescriptorSet;
}

void ShadowRenderSystem::CreatePipeline() {
  assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  Pipeline::defaultPipelineConfigInfo(pipelineConfig);

  pipelineConfig.colorBlendInfo.attachmentCount = 0;
  pipelineConfig.colorBlendInfo.pAttachments = nullptr;

  pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

  // Enable depth bias for better shadow quality
  pipelineConfig.rasterizationInfo.depthBiasEnable = VK_TRUE;
  pipelineConfig.rasterizationInfo.depthBiasConstantFactor = 4.0f;
  pipelineConfig.rasterizationInfo.depthBiasSlopeFactor = 1.5f;
  pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  pipelineConfig.bindingDescriptions = Model::Vertex::getBindingsDescriptions();
  pipelineConfig.attributeDescriptions = Model::Vertex::getAttributeDescriptions();

  pipelineConfig.renderPass = m_renderPass;
  pipelineConfig.pipelineLayout = m_pipelineLayout;
  pipelineConfig.vertPath = "../shader/shadow.vert.spv";
  pipelineConfig.fragPath = "../shader/shadow.frag.spv";

  m_pipeline = std::make_unique<Pipeline>(m_device, pipelineConfig);
}
} // namespace engine