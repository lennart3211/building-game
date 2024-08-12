#include "Application.h"

#include "Camera.h"
#include "Controller.h"
#include "Imgui.h"
#include "systems/ParticleRenderSystem.h"
#include "systems/ParticleSystem.h"
#include "systems/MeshRenderSystem.h"
#include "ResourceManager.h"
#include <descriptors/DescriptorWriter.h>

#include <GLFW/glfw3.h>

#include <chrono>
#include <memory>
#include <random>
#include <iostream>

namespace engine {



    Application::Application() {
        mGlobalPool = DescriptorPool::Builder(mDevice)
                .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();
      }

    Application::~Application() = default;

    void Application::run() {
        Imgui imgui{mWindow, mDevice, mRenderer.GetSwapChainRenderPass(), mRenderer.GetImageCount()};

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("../font/MontserratAlternates-Bold.otf", 32.0f);

        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    mDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(mDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *mGlobalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i]);
        }

        Controlls controlls{
            GLFW_KEY_D,
            GLFW_KEY_A,
            GLFW_KEY_Q,
            GLFW_KEY_E,
            GLFW_KEY_W,
            GLFW_KEY_S,
        };

        const auto player = m_registry.create();
        m_registry.emplace<component::entity_info>(player, "Player");
        m_registry.emplace<component::Camera>(player);
        m_registry.emplace<component::Controller>(player, mWindow, controlls);
        m_registry.emplace<component::transform>(player, glm::vec3(0, -2, -2), glm::vec3(1), glm::vec3(-0.2, 0, 0));

        auto &transform = m_registry.get<component::transform>(player);
        m_registry.get<component::Camera>(player).SetViewYXZ(transform.translation, transform.rotation);

        ResourceManager resourceManager{mDevice};
        resourceManager.importModel("../model/sphere.obj");
        resourceManager.importModel("../model/cube.obj");

        const auto ground = m_registry.create();
        m_registry.emplace<component::entity_info>(ground, "Ground");
        m_registry.emplace<std::shared_ptr<Model>>(ground, resourceManager.getModel("../model/cube.obj"));
        m_registry.emplace<component::transform>(ground, glm::vec3(0), glm::vec3(1, 1, 100), glm::vec3(0));

        const auto car = m_registry.create();
        m_registry.emplace<component::entity_info>(car, "Car");
        m_registry.emplace<std::shared_ptr<Model>>(car, resourceManager.getModel("../model/cube.obj"));
        m_registry.emplace<component::transform>(car, glm::vec3(0, -1.05, 1), glm::vec3(0.2, 0.1, 0.5), glm::vec3(0));

        MeshRenderSystem renderSystem{mDevice,
                                      mRenderer.GetSwapChainRenderPass(),
                                      globalSetLayout->getDescriptorSetLayout(),
                                      "../shader/mesh.vert.spv",
                                      "../shader/mesh.frag.spv"};

        m_backgroundColor = glm::vec3(0.3f, 0.5f, 1.0f);
        mRenderer.SetClearColor(m_backgroundColor);

        auto currentTime = std::chrono::high_resolution_clock::now();

        glm::vec2 prevCursorPosition = mWindow.getCursorPosition();

        std::shared_ptr<Texture> texture;
        VkDescriptorSet textureID{VK_NULL_HANDLE};

        bool inViewport{false};

        while (!mWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            float aspect = mRenderer.GetAspectRatio();
            m_registry.get<component::Camera>(player).SetPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

            if (auto commandBuffer = mRenderer.BeginFrame()) {
                if (texture) textureID = imgui.addTexture(texture->sampler(), texture->imageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                int frameIndex = (int) mRenderer.GetFrameIndex();

                FrameInfo frameInfo{
                        frameIndex,
                        frameTime,
                        commandBuffer,
                        {globalDescriptorSets[frameIndex]},
                      m_registry
                      };

                mRenderer.SetClearColor(m_backgroundColor);

                GlobalUbo ubo{};
                ubo.view = m_registry.get<component::Camera>(player).View();
                ubo.projection = m_registry.get<component::Camera>(player).Projection();
                ubo.ambientLightColor = glm::vec4(m_backgroundColor, 1);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();


//                if (inViewport && mWindow.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
//                  m_registry.view<component::Camera, component::Controller,component::transform>()
//                      .each([&](auto &camera, auto &controller, auto &transform) {
//                            controller.update(transform.translation, transform.rotation, frameTime);
//                            camera.SetViewYXZ(transform.translation, transform.rotation);
//                          });
//                }

                // render
                mRenderer.BeginSwapChainRenderPass(commandBuffer);
                {
                    renderSystem.Render(frameInfo);
                }
                mRenderer.EndSwapChainRenderPass(commandBuffer);

                imgui.newFrame();

                ImGui::Begin("Viewport", nullptr, ImGuiTableColumnFlags_NoResize);
                {
                    inViewport = ImGui::IsWindowHovered();
                    ImVec2 extent = ImGui::GetWindowSize();
                    mRenderer.SetExtent({(uint32_t)extent.x, (uint32_t)extent.y});
                    if (textureID) {
                        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
                        ImGui::Image(textureID, viewportPanelSize);
                    }
                }
                ImGui::End();

                ImGui::Begin("Models");
                {
                    auto names = resourceManager.getModelNames();
                    for (int32_t i = 0; i < names.size(); ++i) {
                        ImGui::PushID(i);
                        ImGui::Text(names[i].c_str());
                        ImGui::PopID();
                    }
                }
                ImGui::End();

                ImGui::Begin("Entities");
                {
                    int32_t i{0};
                    m_registry.view<component::entity_info, component::transform>().each([&](auto &info, auto &transform) {
                      ImGui::PushID(i++);
                      if (ImGui::TreeNode(info.name.c_str())){
                        ImGui::DragFloat3("Position", &transform.translation.x);
                        ImGui::DragFloat3("Scale", &transform.scale.x);
                        ImGui::DragFloat3("Rotation", &transform.rotation.x);
                        ImGui::TreePop();
                      }
                      ImGui::PopID();
                    });
                }
                ImGui::End();

                mRenderer.RenderImGui();
                mRenderer.EndFrame();
                imgui.removeTexture(textureID);
                texture = mRenderer.GetTexture();
            }
        }
    }

    void Application::Update() {}

    void Application::Render() {}

    float Application::frand(float min, float max) {
        static std::mt19937 generator(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_real_distribution<float> distribution(min, max);
        return distribution(generator);
    }

} // namespace engine
