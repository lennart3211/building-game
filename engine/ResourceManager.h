#pragma once

#include "Model.h"
#include "SwapChain.h"
#include "descriptors/DescriptorPool.h"
#include "descriptors/DescriptorSetLayout.h"

#include "Structure.h"
#include "TextureArray.h"
#include "entt/entt.hpp"

namespace engine {

    class ResourceManager {
    private:
       Device &m_device;
       std::shared_ptr<DescriptorPool> m_descriptorPool;
       std::shared_ptr<DescriptorSetLayout> m_descriptorSetLayout;

       std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
       std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
       std::unordered_map<std::string, VkDescriptorSet> m_textureDescritporSets;

    public:
       explicit ResourceManager(Device &device);
       ~ResourceManager();

       bool importModel(const std::string &filepath);
       std::shared_ptr<Model> getModel(const std::string &filepath);
       std::shared_ptr<Model> getModel(Structure::Type type);
       void deleteModel(const std::string &filepath);

       bool importTexture(const std::string &filepath);
       VkDescriptorSet getTexture(const std::string &filepath);
       VkDescriptorSet getTexture(Structure::Type type);
       std::string getTextureName(VkDescriptorSet texture);
       std::vector<std::string> getTextureNames() const;

       std::vector<std::string> getModelNames() const;
       std::string getModelName(const std::shared_ptr<Model> &model);
       VkDescriptorSetLayout getTextureSetLayout() const { return m_descriptorSetLayout->getDescriptorSetLayout(); }
    };

} // engine
