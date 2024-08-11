#include "Application.h"

#include "Camera.h"
#include "Controller.h"
#include "Imgui.h"
#include "systems/ParticleRenderSystem.h"
#include "systems/ParticleSystem.h"
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
        m_registry.emplace<component::Camera>(player);
        m_registry.emplace<component::Controller>(player, mWindow, controlls);
        m_registry.emplace<component::transform>(player, glm::vec3(0), glm::vec3(1), glm::vec3(0));


        ParticleSystem particleSystem{mDevice,
                                      mRenderer.GetSwapChainRenderPass(),
                                      globalSetLayout->getDescriptorSetLayout(),
                                      50,
                                      2};

        const auto p1 = m_registry.create();
        m_registry.emplace<Particle>(p1, glm::vec3(1, 0, 0), glm::vec4(0.9, 0.2, 0.2, 1), 0.05);
        m_registry.emplace<component::physics>(p1, glm::vec3(0, 0, 0), 1.0f, true);

        const auto box = m_registry.create();
        m_registry.emplace<Box>(box, glm::vec3(0), glm::vec3(0.5));

        m_backgroundColor = glm::vec3(0.3f, 0.5f, 1.0f);
        mRenderer.SetClearColor(m_backgroundColor);

        auto currentTime = std::chrono::high_resolution_clock::now();

        glm::vec3 p_position{0};
        glm::vec3 p_velocity{frand(-0.5, 0.5), frand(-0.5, 0.5), frand(-0.5, 0.5)};
        glm::vec4 p_color{frand(0, 1), frand(0, 1), frand(0, 1), 1};
        float p_radius{0.05};
        float p_mass{1};
        bool p_static{false};

        glm::vec2 prevCursorPosition = mWindow.getCursorPosition();

        std::shared_ptr<Texture> texture;
        VkDescriptorSet textureID{VK_NULL_HANDLE};

        ImVec2 particlesSize{500, 0};
        ImVec2 settingsSize{600, 700};
        ImVec2 boxesSize{600, 700};

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

                particleSystem.Update(frameInfo);

                if (inViewport) {
                  m_registry.view<component::Camera, component::Controller,component::transform>()
                      .each([&](auto &camera, auto &controller, auto &transform) {
                            controller.update(transform.translation, transform.rotation, frameTime);
                            camera.SetViewYXZ(transform.translation, transform.rotation);
                          });
                }

                // render
                mRenderer.BeginSwapChainRenderPass(commandBuffer);
                {
                    particleSystem.Render(frameInfo);
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

                ImGui::Begin("Entities");
                {
                    int32_t i{0};
                    m_registry.view<component::transform>().each([&](auto &transform) {
                      ImGui::PushID(i++);
                      if (ImGui::TreeNode(("Entity " + std::to_string(i)).c_str())){
                        ImGui::DragFloat3("Position", &transform.translation.x);
                        ImGui::DragFloat3("Rotation", &transform.rotation.x);
                        ImGui::TreePop();
                      }
                      ImGui::PopID();
                    });
                }
                ImGui::End();

                ImGui::SetNextWindowPos({(float)mWindow.width() - boxesSize.x, settingsSize.y});
                ImGui::SetNextWindowSize({boxesSize.x, boxesSize.y});
                ImGui::Begin("Boxes");
                {
                    m_registry.view<Box>().each([&](auto e, auto &box) {
                      ImGui::PushID((int32_t)e);
                      ImGui::DragFloat3("Position", &box.position.x, 0.05);
                      ImGui::DragFloat3("Half Extent", &box.halfExtent.x, 0.05);
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
