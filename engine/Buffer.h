#pragma once

#include "Device.h"

namespace engine {

    class Buffer {
    public:
        Buffer(
                Device &device,
                VkDeviceSize instanceSize,
                uint32_t instanceCount,
                VkBufferUsageFlags usageFlags,
                VkMemoryPropertyFlags memoryPropertyFlags,
                VkDeviceSize minOffsetAlignment = 1);

        ~Buffer();

        Buffer(const Buffer &) = delete;

        Buffer &operator=(const Buffer &) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void unmap();

        void writeToBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void writeToIndex(void *data, int index);

        VkResult flushIndex(int index);

        VkDescriptorBufferInfo descriptorInfoForIndex(int index);

        VkResult invalidateIndex(int index);

        [[nodiscard]] VkBuffer getBuffer() const { return _buffer; }

        [[nodiscard]] void *getMappedMemory() const { return _mapped; }

        [[nodiscard]] uint32_t getInstanceCount() const { return _instanceCount; }

        [[nodiscard]] VkDeviceSize getInstanceSize() const { return _instanceSize; }

        [[nodiscard]] VkDeviceSize getAlignmentSize() const { return _instanceSize; }

        [[nodiscard]] VkBufferUsageFlags getUsageFlags() const { return _usageFlags; }

        [[nodiscard]] VkMemoryPropertyFlags getMemoryPropertyFlags() const { return _memoryPropertyFlags; }

        [[nodiscard]] VkDeviceSize getBufferSize() const { return _bufferSize; }

        [[nodiscard]] VkDeviceAddress getBufferDeviceAddress() const {
            VkBufferDeviceAddressInfo addressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            addressInfo.buffer = _buffer;
            return vkGetBufferDeviceAddress(_device.device(), &addressInfo);
        }


    private:
        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        Device &_device;
        void *_mapped = nullptr;
        VkBuffer _buffer = VK_NULL_HANDLE;
        VkDeviceMemory _memory = VK_NULL_HANDLE;

        VkDeviceSize _bufferSize;
        uint32_t _instanceCount;
        VkDeviceSize _instanceSize;
        VkDeviceSize _alignmentSize;
        VkBufferUsageFlags _usageFlags;
        VkMemoryPropertyFlags _memoryPropertyFlags;
    };

}  // namespace engine