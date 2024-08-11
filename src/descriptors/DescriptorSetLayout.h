#pragma once

#include "Device.h"

#include <memory>
#include <unordered_map>

namespace engine {
class DescriptorSetLayout {
public:
    class Builder {
    public:
        Builder(Device &device) : mDevice{device} {}

        Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);

        std::unique_ptr <DescriptorSetLayout> build() const;

    private:
        Device &mDevice;
        std::unordered_map <uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    DescriptorSetLayout(
            Device &device, std::unordered_map <uint32_t, VkDescriptorSetLayoutBinding> bindings);

    ~DescriptorSetLayout();

    DescriptorSetLayout(const DescriptorSetLayout &) = delete;

    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const { return mDescriptorSetLayout; }

private:
    Device &mDevice;
    VkDescriptorSetLayout mDescriptorSetLayout;
    std::unordered_map <uint32_t, VkDescriptorSetLayoutBinding> mBindings;

    friend class DescriptorWriter;
};

}