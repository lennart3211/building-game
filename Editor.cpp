#include "Editor.h"

#include "Camera.h"
#include "Imgui.h"
#include "TextureArray.h"
#include "descriptors/DescriptorWriter.h"
#include "systems/MeshRenderSystem.h"

#include "imgui/imgui_stdlib.h"

#include "GLFW/glfw3.h"
#include "systems/ShadowRenderSystem.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <random>

namespace engine {

  Editor::Editor() {
    mGlobalPool = DescriptorPool::Builder(m_device)
                      .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                   SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                   SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .build();
  }

  Editor::~Editor() = default;

  void Editor::run() {
    Imgui imgui{m_window, m_device, m_renderer.GetSwapChainRenderPass(),
                m_renderer.GetImageCount()};

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../font/MontserratAlternates-Bold.otf",
                                 32.0f);

    std::vector<std::unique_ptr<Buffer>> uboBuffers(
        SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto &uboBuffer : uboBuffers) {
      uboBuffer = std::make_unique<Buffer>(m_device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      uboBuffer->map();
    }

    auto globalSetLayout =
        DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(
        SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (uint32_t i = 0; i < globalDescriptorSets.size(); i++) {
      auto bufferInfo = uboBuffers[i]->descriptorInfo();
      DescriptorWriter(*globalSetLayout, *mGlobalPool)
          .writeBuffer(0, &bufferInfo)
          .build(globalDescriptorSets[i]);
    }

    ShadowRenderSystem shadowRenderSystem{m_device, globalSetLayout->getDescriptorSetLayout()};

    MeshRenderSystem renderSystem{m_device,
                                  m_renderer.GetSwapChainRenderPass(),
                                  {
                                      globalSetLayout->getDescriptorSetLayout(),
                                      shadowRenderSystem.GetDescriptorSetLayout(),
                                      m_resourceManager.getTextureSetLayout()
                                  },
                                  "../shader/mesh.vert.spv",
                                  "../shader/mesh.frag.spv"
    };


    m_backgroundColor = glm::vec3(0.3f, 0.5f, 1.0f);
    m_renderer.SetClearColor(m_backgroundColor);

    auto currentTime = std::chrono::high_resolution_clock::now();

    glm::vec2 prevCursorPosition = m_window.getCursorPosition();

    std::shared_ptr<Texture> texture;
    VkDescriptorSet textureID{VK_NULL_HANDLE};

    component::Camera cam;
    auto camPos = glm::vec3{0, -5, 0};
    cam.SetViewTarget(camPos, glm::vec3{5, 0, 5});

    m_resourceManager.importModel("../model/structure_1.obj");
    m_resourceManager.importTexture("../textures/structure_1.png");

    m_game.PlaceStructure({5, 0, 5}, Structure::COLOR_1);
//    m_game.PlaceStructure({0, 1, 0}, Structure::COLOR_1);

    float placeTimeout = 1.0f;
    float placeTimer = 0.0f;

    while (!m_window.shouldClose()) {
      glfwPollEvents();

      if (m_window.minimized()) {
        continue;
      }

      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime =
          std::chrono::duration<float, std::chrono::seconds::period>(
              newTime - currentTime)
              .count();
      currentTime = newTime;

      float aspect = m_renderer.GetAspectRatio();
      cam.SetPerspectiveProjection(
          glm::radians(50.0f), aspect, 0.1f, 100.0f);

      if (placeTimer > 0.0f) {
        placeTimer -= frameTime;
      }

      if (auto commandBuffer = m_renderer.BeginFrame()) {
        if (texture)
          textureID =
              imgui.addTexture(texture->sampler(), texture->imageView(),
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        int frameIndex = (int)m_renderer.GetFrameIndex();

        GlobalUbo ubo{};
        ubo.view = cam.View();
        ubo.projection = cam.Projection();
        ubo.ambientLightColor = glm::vec4(m_backgroundColor, 1);

        ubo.lightPosition = glm::vec3{-5.0f, -10.0f, -5.0f};

        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);

        glm::mat4 lightView = glm::lookAt(
            glm::vec3(ubo.lightPosition),
            glm::vec3(0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)
        );

        ubo.lightSpaceMatrix = lightProjection * lightView;

        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        FrameInfo frameInfo{frameIndex,
                            frameTime,
                            commandBuffer,
                            {globalDescriptorSets[frameIndex]},
                            m_game.GetStructures(),
                            m_resourceManager
        };

        m_renderer.SetClearColor(m_backgroundColor);

        // render
        shadowRenderSystem.Render(frameInfo);

        VkMemoryBarrier memoryBarrier{};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            1, &memoryBarrier,
            0, nullptr,
            0, nullptr
        );

        m_renderer.BeginSwapChainRenderPass(commandBuffer);
        {
          frameInfo.descriptorSets.push_back(shadowRenderSystem.GetShadowMapDescriptorSet());
          renderSystem.Render(frameInfo);
        }
        m_renderer.EndSwapChainRenderPass(commandBuffer);

        imgui.newFrame();

        ImGui::Begin("Viewport", nullptr, ImGuiTableColumnFlags_NoResize);
        {
          ImVec2 extent = ImGui::GetWindowSize();
          m_renderer.SetExtent({(uint32_t)extent.x, (uint32_t)extent.y});

          if (textureID) {
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            ImGui::Image((ImTextureID)textureID, viewportPanelSize);

            if (ImGui::IsItemHovered()) {
              auto direction = getCursorRayOriginDirection(cam);
              auto [pos1, pos2, hit] = m_game.intersectsStructure(camPos, direction);

              if (hit && placeTimer <= 0.0f) {
                if (m_window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                  m_game.PlaceStructure(pos2, Structure::Color((uint32_t(frand(0, 1) * 100) % Structure::COLOR_MAX)));
                  placeTimer = placeTimeout;
                }
              }
            }
          }
        }
        ImGui::End();

      }
      m_renderer.RenderImGui();
      m_renderer.EndFrame();
      imgui.removeTexture(textureID);

      texture = m_renderer.GetTexture();
    }

  }

  float Editor::frand(float min, float max) {
    static std::mt19937 generator(
        static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
  }

  glm::vec3 Editor::getCursorRayOriginDirection(const component::Camera& camera) {
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageMax = ImGui::GetItemRectMax();

    glm::vec4 rayClip{(2.0f * (mousePos.x - imageMin.x)) / (imageMax.x - imageMin.x) - 1.0f, 1.0f - (2.0f * (mousePos.y - imageMin.y)) / (imageMax.y - imageMin.y), 1.0f, 1.0f};

    glm::vec4 rayEye = glm::inverse(camera.Projection()) * rayClip;
    rayEye.z = 1.0f;
    rayEye.w = 0.0f;


     glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(camera.View()) * rayEye));

    return rayWorld;
  }


} // namespace engine