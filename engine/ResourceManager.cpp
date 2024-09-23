//
// Created by lennart on 7/31/24.
//

#include "ResourceManager.h"
#include "descriptors/DescriptorWriter.h"

#include <filesystem>


namespace engine {
ResourceManager::ResourceManager(Device &device) : m_device(device) {
  m_descriptorPool = DescriptorPool::Builder(m_device)
                    .setMaxSets(1000)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                    .build();

  m_descriptorSetLayout = DescriptorSetLayout::Builder{m_device}.
                          addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
                          build();

}

bool ResourceManager::importModel(const std::string &filepath) {

  std::string name = std::filesystem::path(filepath).filename().string();
  if (m_models.find(name) != m_models.end()) return false;

  Model::Builder builder{};
  if (builder.LoadModel(filepath)) {
    m_models[name] = std::make_shared<Model>(m_device, builder);
    return true;
  }
  return false;
}

std::shared_ptr<Model> ResourceManager::getModel(const std::string &filepath) {
  return m_models[filepath];
}

bool ResourceManager::importTexture(const std::string &filepath) {
    std::string name = std::filesystem::path(filepath).filename().string();
    if (m_textures.find(name) == m_textures.end()) {
      m_textures[name] = std::make_shared<Texture>(m_device, filepath);
      return true;
    }
    return false;
}

VkDescriptorSet ResourceManager::getTexture(const std::string &filepath) {
    if (m_textureDescritporSets.find(filepath) != m_textureDescritporSets.end()) {
        return m_textureDescritporSets[filepath];
    }
    auto it = m_textures.find(filepath);
    if (it == m_textures.end()) {
      throw std::runtime_error("Texture not found: " + filepath);
    }
    auto &texture = it->second;
    VkDescriptorSet descriptorSet;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = texture->sampler();
    imageInfo.imageView = texture->imageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    DescriptorWriter{*m_descriptorSetLayout, *m_descriptorPool}
        .writeImage(0, &imageInfo, 1)
        .build(descriptorSet);

    m_textureDescritporSets[filepath] = descriptorSet;

    return descriptorSet;
}

std::vector<std::string> ResourceManager::getModelNames() const {
    std::vector<std::string> names;
    names.reserve(m_models.size());
    for (const auto & model : m_models) {
        names.push_back(model.first);
    }
    return names;
}

ResourceManager::~ResourceManager() = default;

std::string ResourceManager::getModelName(const std::shared_ptr<Model> &model) {
    for (auto &it : m_models) {
        if (it.second == model) {
          return it.first;
        }
    }
    return {};
}

void ResourceManager::deleteModel(const std::string &filepath) {
    m_models.erase(filepath);
}

std::string ResourceManager::getTextureName(VkDescriptorSet texture) {
    for (auto &it : m_textureDescritporSets) {
        if (it.second == texture) {
          return it.first;
        }
    }
    return {};
}

std::vector<std::string> ResourceManager::getTextureNames() const {
    std::vector<std::string> names;
    names.reserve(m_textures.size());
    for (const auto & texture : m_textures) {
        names.push_back(texture.first);
    }
    return names;
}

} // engine