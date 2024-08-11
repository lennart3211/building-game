#pragma once

#include "Device.h"

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace engine {

    struct PipelineConfigInfo {
        PipelineConfigInfo(const PipelineConfigInfo &) = delete;

        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        std::string vertPath;
        std::string fragPath;
        std::string geomPath;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
    public:
        Pipeline(Device &device, const PipelineConfigInfo &configInfo);

        ~Pipeline();

        Pipeline(const Pipeline &) = delete;

        void operator=(const Pipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

    private:
        static std::vector<char> readFile(const std::string &filepath);

        void createGraphicsPipeline(const PipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code,
                                VkShaderModule *shaderModule);

        Device &m_Device;
        VkPipeline m_GraphicsPipeline;
        VkShaderModule m_VertShaderModule{VK_NULL_HANDLE};
        VkShaderModule m_FragShaderModule{VK_NULL_HANDLE};
        VkShaderModule m_geomShaderModule{VK_NULL_HANDLE};
    };
} // namespace engine
