#pragma once

#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"

namespace engine {
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

        DescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo, int descriptorCount = 1);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        DescriptorSetLayout &mSetLayout;
        DescriptorPool &mPool;
        std::vector<VkWriteDescriptorSet> mWrites;
    };
}