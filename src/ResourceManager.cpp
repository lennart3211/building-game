//
// Created by lennart on 7/31/24.
//

#include "ResourceManager.h"

namespace engine {
ResourceManager::ResourceManager(Device &device) : m_device(device) {
  m_descriptorPool = DescriptorPool::Builder(m_device)
                    .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT * 10)
                    .build();
}

void ResourceManager::importModel(const std::string &filepath) {
  Model::Builder builder{};
  builder.LoadModel(filepath);
  m_models[filepath] = std::make_shared<Model>(m_device, builder);
}

std::shared_ptr<Model> ResourceManager::getModel(const std::string &filepath) {
  return m_models[filepath];
}

void ResourceManager::importTexture(const std::string &filepath) {

}

VkDescriptorSet ResourceManager::getTexture(const std::string &filepath) {

}

    std::vector<std::string> ResourceManager::getModelNames() const {
        std::vector<std::string> names;
        for (const auto & m_model : m_models) {
            names.push_back(m_model.first);
        }
        return names;
    }


} // engine