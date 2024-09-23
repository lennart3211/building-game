//
// Created by lennart on 4/14/24.
//

#include <stdexcept>
#include "DescriptorPool.h"

namespace engine {
DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
    mPoolSizes.push_back({descriptorType, count});
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
    mPoolFlags = flags;
    return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
    mMaxSets = count;
    return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
    return std::make_unique<DescriptorPool>(mDevice, mMaxSets, mPoolFlags, mPoolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(
        Device &device,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize> &poolSizes)
        : mDevice{device} {
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(mDevice.device(), &descriptorPoolInfo, nullptr, &mDescriptorPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(mDevice.device(), mDescriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(mDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
    vkFreeDescriptorSets(
            mDevice.device(),
            mDescriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
}

void DescriptorPool::resetPool() {
    vkResetDescriptorPool(mDevice.device(), mDescriptorPool, 0);
}

}