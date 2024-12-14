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

#include "Structure.h"

#define GLM_FORCE_RADIANS
#define GLM_FROCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#include "Camera.h"
#include "Game.h"
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

        Window m_window{WIDTH, HEIGHT, "App"};
        Device m_device{m_window};
        Renderer m_renderer{m_window, m_device, {100, 100}};
        ResourceManager m_resourceManager{m_device};
        FileManager m_fileManager;

        // Declaration order matters!!!!!!
        std::unique_ptr<DescriptorPool> mGlobalPool{};

        Game<10, 5, 10> m_game;
        glm::vec3 m_backgroundColor;

    public:
      Editor();
        ~Editor();
        Editor(const Editor &) = delete;
        Editor &operator=(const Editor &) = delete;

        void run();

    private:

        static float frand(float min, float max);

        glm::vec3 getCursorRayOriginDirection(const component::Camera& camera);
    };
} // namespace engine
