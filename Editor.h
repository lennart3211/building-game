#pragma once

#include "Device.h"
#include "Pipeline.h"
#include "Window.h"
#include "Renderer.h"
#include "Buffer.h"
#include "descriptors/DescriptorPool.h"
#include "Components.h"

#include "FileManager.h"
#include "ResourceManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#define GLM_FORCE_RADIANS
#define GLM_FROCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include "entt/entt.hpp"
#include <entt/entity/registry.hpp>

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace engine {



    class Editor {
    private:
        static constexpr uint16_t WIDTH = 1920;
        static constexpr uint16_t HEIGHT = 1920;
        const std::string MODEL_BASE_PATH{"../model/"};

        struct assign_info {
          entt::entity entity;
          std::string name;
        };

        struct displayEntities_payload {
            assign_info assignModel;
            assign_info assignTexture;
        };

        Window mWindow{WIDTH, HEIGHT, "App"};
        Device mDevice{mWindow};
        Renderer mRenderer{mWindow, mDevice, {100, 100}};
        ResourceManager m_resourceManager{mDevice};
        FileManager m_fileManager;

        // Declaration order matters!!!!!!
        std::unique_ptr<DescriptorPool> mGlobalPool{};

        glm::vec3 m_backgroundColor;
        glm::vec4 m_snakeColor;

        entt::registry m_registry;
        std::vector<component::entity_info> m_entities;

    public:
      Editor();
        ~Editor();
        Editor(const Editor &) = delete;
        Editor &operator=(const Editor &) = delete;

        void run();

    private:
        void Update();
        void Render();

        std::pair<uint32_t, std::string> displayModels(std::string &modelPath);
        std::pair<uint32_t, std::string>
        displayTextures(std::string &texturePath);
        Editor::displayEntities_payload displayEntities(const ImVec2 &&pos,
                                                        const ImVec2 &&size);

        static float frand(float min, float max);
    };
} // namespace engine
