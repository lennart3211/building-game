//
// Created by lennart on 4/15/24.
//

#include "DescriptorWriter.h"

#include <cassert>

namespace engine {
DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool)
        : mSetLayout{setLayout}, mPool{pool} {}

DescriptorWriter &DescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
    assert(mSetLayout.mBindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = mSetLayout.mBindings[binding];

    assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    mWrites.push_back(write);
    return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo *imageInfo, int descriptorCount) {
    assert(mSetLayout.mBindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &bindingDescription = mSetLayout.mBindings[binding];

//    assert(
//            bindingDescription.descriptorCount == 1 &&
//            "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = descriptorCount;

    mWrites.push_back(write);
    return *this;
}

bool DescriptorWriter::build(VkDescriptorSet &set) {
    bool success = mPool.allocateDescriptor(mSetLayout.getDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet &set) {
    for (auto &write : mWrites) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(mPool.mDevice.device(), mWrites.size(), mWrites.data(), 0, nullptr);
}

}
