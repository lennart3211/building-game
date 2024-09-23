//
// Created by mclen on 01/08/2024.
//

#include "Controller.h"
#include "glm/common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/constants.hpp"
#include <iostream>

namespace engine::component {
void Controller::update(glm::vec3 &position, glm::vec3 &rotation, float dt) {
  if (m_controlls.condition != GLFW_KEY_UNKNOWN && !m_window.isMouseButtonPressed(m_controlls.condition)) {
    m_prevCursorPos = m_window.getCursorPosition();
    return;
  }

  // rotation
  glm::vec2 currCursorPos = m_window.getCursorPosition();
  glm::vec2 dCursor = currCursorPos - m_prevCursorPos;


  rotation.y -= dCursor.x * m_rotationSpeed;
  rotation.x += dCursor.y * m_rotationSpeed;

  rotation.x = glm::clamp(rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());

  m_prevCursorPos = currCursorPos;

  // movement
  glm::vec3 moveDir(0.0f);
  if (m_window.isKeyPressed(m_controlls.moveZPos)) moveDir.z += 1.0f;
  if (m_window.isKeyPressed(m_controlls.moveZNeg)) moveDir.z -= 1.0f;
  if (m_window.isKeyPressed(m_controlls.moveXNeg)) moveDir.x -= 1.0f;
  if (m_window.isKeyPressed(m_controlls.moveXPos)) moveDir.x += 1.0f;
  if (m_window.isKeyPressed(m_controlls.moveYPos)) moveDir.y += 1.0f;
  if (m_window.isKeyPressed(m_controlls.moveYNeg)) moveDir.y -= 1.0f;

  if (glm::length(moveDir) > 0.0f) {
    moveDir = glm::normalize(moveDir);
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
    glm::vec3 rotatedMoveDir = glm::vec3(rotationMatrix * glm::vec4(moveDir, 0.0f));
    position += rotatedMoveDir * m_movementSpeed * dt;
  }

}
} // namespace engine::component