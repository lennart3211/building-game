#pragma once

#include "Device.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace engine {
class DescriptorPool {
public:
    class Builder {
    public:
        Builder(Device &device) : mDevice{device} {}

        Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);

        Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);

        Builder &setMaxSets(uint32_t count);

        std::unique_ptr<DescriptorPool> build() const;

    private:
        Device &mDevice;
        std::vector<VkDescriptorPoolSize> mPoolSizes{};
        uint32_t mMaxSets = 1000;
        VkDescriptorPoolCreateFlags mPoolFlags = 0;
    };

    DescriptorPool(
            Device &device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);

    ~DescriptorPool();

    DescriptorPool(const DescriptorPool &) = delete;

    DescriptorPool &operator=(const DescriptorPool &) = delete;

    bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

    void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

    void resetPool();

private:
    Device &mDevice;
    VkDescriptorPool mDescriptorPool;

    friend class DescriptorWriter;
};

}