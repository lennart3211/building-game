#include "Editor.h"

#include "Camera.h"
#include "Controller.h"
#include "Imgui.h"
#include "TextureArray.h"
#include "descriptors/DescriptorWriter.h"
#include "systems/MeshRenderSystem.h"
#include "systems/ParticleRenderSystem.h"

#include "imgui/imgui_stdlib.h"

#include "GLFW/glfw3.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <random>

namespace engine {

  Editor::Editor() {
    mGlobalPool = DescriptorPool::Builder(mDevice)
                      .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                   SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                   SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .build();
  }

  Editor::~Editor() = default;

  void Editor::run() {
    Imgui imgui{mWindow, mDevice, mRenderer.GetSwapChainRenderPass(),
                mRenderer.GetImageCount()};

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../font/MontserratAlternates-Bold.otf",
                                 32.0f);

    std::vector<std::unique_ptr<Buffer>> uboBuffers(
        SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto &uboBuffer : uboBuffers) {
      uboBuffer = std::make_unique<Buffer>(
          mDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      uboBuffer->map();
    }

    auto globalSetLayout =
        DescriptorSetLayout::Builder(mDevice)
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

    Controlls controlls{
        GLFW_KEY_D, GLFW_KEY_A, GLFW_KEY_Q,
        GLFW_KEY_E, GLFW_KEY_W, GLFW_KEY_S,
    };

    std::string carModelPath = "../model/cube.obj";

    const auto player = m_registry.create();
    m_registry.emplace<component::entity_info>(player, player, "Viewer");
    m_registry.emplace<component::Camera>(player);
    m_registry.emplace<component::Controller>(player, mWindow, controlls);
    m_registry.emplace<component::transform>(
        player, glm::vec3(0, -2, -2), glm::vec3(1), glm::vec3(-0.2, 0, 0));

    auto &transform = m_registry.get<component::transform>(player);
    m_registry.get<component::Camera>(player).SetViewYXZ(
        transform.translation, transform.rotation);

    MeshRenderSystem renderSystem{mDevice,
                                  mRenderer.GetSwapChainRenderPass(),
                                  {globalSetLayout->getDescriptorSetLayout(),
                                   m_resourceManager.getTextureSetLayout()},
                                  "../shader/mesh.vert.spv",
                                  "../shader/mesh.frag.spv"};

    m_backgroundColor = glm::vec3(0.3f, 0.5f, 1.0f);
    mRenderer.SetClearColor(m_backgroundColor);

    auto currentTime = std::chrono::high_resolution_clock::now();

    glm::vec2 prevCursorPosition = mWindow.getCursorPosition();

    std::shared_ptr<Texture> texture;
    VkDescriptorSet textureID{VK_NULL_HANDLE};

    bool inViewport{false};

    auto modelNames = m_resourceManager.getModelNames();

    std::string modelPath = "../model/";
    std::string texturePath = "../textures/";

    std::pair<uint32_t, std::string> deleteModel = {0, {}};
    std::pair<uint32_t, std::string> deleteTexture = {0, {}};
    assign_info assignModel = {entt::entity(0), {}};
    assign_info assignTexture = {entt::entity(0), {}};

    while (!mWindow.shouldClose()) {
      glfwPollEvents();

      if (mWindow.minimized()) {
        continue;
      }

      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime =
          std::chrono::duration<float, std::chrono::seconds::period>(
              newTime - currentTime)
              .count();
      currentTime = newTime;

      float aspect = mRenderer.GetAspectRatio();
      m_registry.get<component::Camera>(player).SetPerspectiveProjection(
          glm::radians(50.0f), aspect, 0.1f, 100.0f);

      if (auto commandBuffer = mRenderer.BeginFrame()) {
        if (texture)
          textureID =
              imgui.addTexture(texture->sampler(), texture->imageView(),
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        int frameIndex = (int)mRenderer.GetFrameIndex();

        FrameInfo frameInfo{frameIndex,
                            frameTime,
                            commandBuffer,
                            {globalDescriptorSets[frameIndex]},
                            m_registry};

        mRenderer.SetClearColor(m_backgroundColor);

        GlobalUbo ubo{};
        ubo.view = m_registry.get<component::Camera>(player).View();
        ubo.projection =
            m_registry.get<component::Camera>(player).Projection();
        ubo.ambientLightColor = glm::vec4(m_backgroundColor, 1);

        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        if (inViewport) {
          m_registry.view<component::Controller, component::transform>().each(
              [&](auto &controller, auto &transform) {
                auto rotation = glm::vec3(-0.2, 0, 0);
                controller.update(transform.translation, rotation, frameTime);
              });
        }

        if (!deleteModel.second.empty()) {
          m_resourceManager.deleteModel(modelNames[deleteModel.first]);
          modelNames.erase(modelNames.begin() + deleteModel.first);
        }

        if (!assignModel.name.empty()) {
          m_registry.remove<std::shared_ptr<Model>>(assignModel.entity);
          m_registry.emplace<std::shared_ptr<Model>>(
              assignModel.entity,
              m_resourceManager.getModel(assignModel.name));
        }

        if (!assignTexture.name.empty()) {
          m_registry.remove<component::material>(assignTexture.entity);
          m_registry.emplace<component::material>(
              assignTexture.entity,
              m_resourceManager.getTexture(assignTexture.name));
          std::cout << "Texture assigned\n";
        }

        modelNames = m_resourceManager.getModelNames();

        // render
        mRenderer.BeginSwapChainRenderPass(commandBuffer);
        { renderSystem.Render(frameInfo); }
        mRenderer.EndSwapChainRenderPass(commandBuffer);

        imgui.newFrame();

        ImGui::Begin("Viewport", nullptr, ImGuiTableColumnFlags_NoResize);
        {
          inViewport = ImGui::IsWindowHovered();
          ImVec2 extent = ImGui::GetWindowSize();
          mRenderer.SetExtent({(uint32_t)extent.x, (uint32_t)extent.y});
          if (textureID) {
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            ImGui::Image((ImTextureID)textureID, viewportPanelSize);
          }
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize({(float)mWindow.width(), 0});
        ImGui::Begin("Menu", nullptr,
                     ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar);
        {
          if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
              if (ImGui::MenuItem("Open",
                                  "Ctrl+O")) { /* Handle open action */
              }
              if (ImGui::MenuItem("Save",
                                  "Ctrl+S")) { /* Handle save action */
              }
              if (ImGui::BeginMenu("Import", "Ctrl+I")) {
                if (ImGui::MenuItem("Model", "Ctrl+M")) {
                  std::vector<nfdu8filteritem_t> filters{{"OBJ", "obj"}};
                  std::string path = m_fileManager.open(filters);
                  m_resourceManager.importModel(path);
                }
                if (ImGui::MenuItem("Texture", "Ctrl+T")) {
                  std::vector<nfdu8filteritem_t> filters{
                      {"Image Files", "png, jpg, jpeg"}};
                  std::string path = m_fileManager.open(filters);
                  m_resourceManager.importTexture(path);
                }
                ImGui::EndMenu();
              }
              ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
              if (ImGui::MenuItem("Undo",
                                  "Ctrl+Z")) { /* Handle undo action */
              }
              if (ImGui::MenuItem("Redo",
                                  "Ctrl+Y")) { /* Handle redo action */
              }
              ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
          }
        }
        ImGui::End();

        deleteModel = displayModels(modelPath);
        deleteTexture = displayTextures(texturePath);
        displayEntities_payload payload = displayEntities(
            {io.DisplaySize.x - 500, 50}, {500, io.DisplaySize.y - 50});
        assignModel = payload.assignModel;
        assignTexture = payload.assignTexture;

        mRenderer.RenderImGui();
        mRenderer.EndFrame();
        imgui.removeTexture(textureID);

        texture = mRenderer.GetTexture();
      }
    }
  }

  void Editor::Update() {}

  void Editor::Render() {}

  float Editor::frand(float min, float max) {
    static std::mt19937 generator(
        static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
  }

  std::pair<uint32_t, std::string> Editor::displayModels(std::string &
                                                         modelPath) {
    std::pair<uint32_t, std::string> deleteModel{0, {}};
    std::vector<std::string> modelNames = m_resourceManager.getModelNames();

    ImGui::Begin("Models");
    {
      for (int32_t i = 0; i < modelNames.size(); ++i) {
        ImGui::PushID(i);
        ImGui::Button(modelNames[i].c_str());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
          std::string item_id = modelNames[i];
          ImGui::SetDragDropPayload("ASSIGN_MODEL", &item_id,
                                    sizeof(std::string));
          ImGui::Text(item_id.c_str());

          ImGui::EndDragDropSource();
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
          std::vector<entt::entity> entities;
          m_registry.view<std::shared_ptr<Model>>().each(
              [&](auto e, auto &model) {
                if (modelNames[i] == m_resourceManager.getModelName(model)) {
                  entities.push_back(e);
                }
              });
          for (auto entity : entities) {
            m_registry.remove<std::shared_ptr<Model>>(entity);
          }
          deleteModel.first = i;
          deleteModel.second = modelNames[i];
        }
        ImGui::PopID();
      }
    }
    ImGui::End();
    return deleteModel;
  }

  Editor::displayEntities_payload Editor::displayEntities(
      const ImVec2 &&pos, const ImVec2 &&size) {
    displayEntities_payload returnVal;

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("Entities");
    {
      int32_t i{0};
      m_registry.view<component::entity_info, component::transform>().each(
          [&](auto &info, auto &transform) {
            ImGui::PushID(i++);
            if (ImGui::TreeNode(info.name.c_str())) {
              if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("ASSIGN_MODEL")) {
                  IM_ASSERT(payload->DataSize == sizeof(std::string));
                  std::string payload_item_id = *(std::string *)payload->Data;
                  ImGui::Text(payload_item_id.c_str());
                  returnVal.assignModel.entity = info.entity;
                  returnVal.assignModel.name = payload_item_id;
                }
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("ASSIGN_TEXTURE")) {
                  IM_ASSERT(payload->DataSize == sizeof(std::string));
                  std::string payload_item_id = *(std::string *)payload->Data;
                  ImGui::Text(payload_item_id.c_str());
                  returnVal.assignTexture.entity = info.entity;
                  returnVal.assignTexture.name = payload_item_id;
                }
                ImGui::EndDragDropTarget();
              }
              if (ImGui::TreeNode("Transform")) {
                ImGui::InputFloat3("Position", &transform.translation.x);
                ImGui::InputFloat3("Scale", &transform.scale.x);
                ImGui::InputFloat3("Rotation", &transform.rotation.x);
                ImGui::TreePop();
              }

              if (m_registry.all_of<component::material>(info.entity) &&
                  ImGui::TreeNode("Material")) {
                auto &material =
                    m_registry.get<component::material>(info.entity);
                ImGui::Text(&m_resourceManager.getTextureName(
                    material.descriptorSet)[0]);
                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                  m_registry.remove<component::material>(info.entity);
                }
                ImGui::TreePop();
              }

              if (m_registry.all_of<std::shared_ptr<Model>>(info.entity) &&
                  ImGui::TreeNode("Model")) {
                auto &model =
                    m_registry.get<std::shared_ptr<Model>>(info.entity);
                ImGui::Text(&m_resourceManager.getModelName(model)[0]);
                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                  m_registry.remove<std::shared_ptr<Model>>(info.entity);
                }
                ImGui::TreePop();
              }

              ImGui::TreePop();

            } else {
              if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("ASSIGN_MODEL")) {
                  IM_ASSERT(payload->DataSize == sizeof(std::string));
                  std::string payload_item_id = *(std::string *)payload->Data;
                  ImGui::Text(payload_item_id.c_str());
                  returnVal.assignModel.entity = info.entity;
                  returnVal.assignModel.name = payload_item_id;
                }
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("ASSIGN_TEXTURE")) {
                  IM_ASSERT(payload->DataSize == sizeof(std::string));
                  std::string payload_item_id = *(std::string *)payload->Data;
                  ImGui::Text(payload_item_id.c_str());
                  returnVal.assignTexture.entity = info.entity;
                  returnVal.assignTexture.name = payload_item_id;
                }
                ImGui::EndDragDropTarget();
              }
            }

            ImGui::PopID();
          });

      if (ImGui::Button("New")) {
        const auto e = m_registry.create();
        m_registry.emplace<component::entity_info>(
            e, e, "Entity " + std::to_string((uint32_t)e));
        m_registry.emplace<component::transform>(e, glm::vec3(0),
                                                 glm::vec3(1), glm::vec3(0));
      }
    }
    ImGui::End();

    return returnVal;
  }

  std::pair<uint32_t, std::string> Editor::displayTextures(std::string &
                                                           texturePath) {
    std::pair<uint32_t, std::string> deleteTexture{0, {}};
    std::vector<std::string> textureNames =
        m_resourceManager.getTextureNames();

    ImGui::Begin("Textures");
    {
      for (int32_t i = 0; i < textureNames.size(); ++i) {
        ImGui::PushID(i);
        ImGui::Button(textureNames[i].c_str());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
          std::string item_id = textureNames[i];
          ImGui::SetDragDropPayload("ASSIGN_TEXTURE", &item_id,
                                    sizeof(std::string));
          ImGui::Text(item_id.c_str());

          ImGui::EndDragDropSource();
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
          std::vector<entt::entity> entities;
          m_registry.view<component::material>().each([&](auto e,
                                                          auto &material) {
            if (textureNames[i] ==
                m_resourceManager.getTextureName(material.descriptorSet)) {
              entities.push_back(e);
            }
          });
          for (auto entity : entities) {
            m_registry.remove<std::shared_ptr<Model>>(entity);
          }
          deleteTexture.first = i;
          deleteTexture.second = textureNames[i];
        }
        ImGui::PopID();
      }
    }
    ImGui::End();
    return deleteTexture;
  }

} // namespace engine