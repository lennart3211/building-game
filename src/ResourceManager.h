#pragma once

#include "Model.h"
#include "SwapChain.h"
#include "descriptors/DescriptorPool.h"
#include "descriptors/DescriptorSetLayout.h"

#include "entt/entt.hpp"

namespace engine {

    class ResourceManager {
    private:
       Device &m_device;
       std::unique_ptr<DescriptorPool> m_descriptorPool;
       std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
       std::vector<VkDescriptorSet> m_descriptorSets{SwapChain::MAX_FRAMES_IN_FLIGHT};

       std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
       std::unordered_map<std::string, VkDescriptorSet> m_textures;

    public:
       explicit ResourceManager(Device &device);

       void importModel(const std::string &filepath);
       std::shared_ptr<Model> getModel(const std::string &filepath);

       void importTexture(const std::string &filepath);
       VkDescriptorSet getTexture(const std::string &filepath);
    };

} // engine
