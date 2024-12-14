//
// Created by mclen on 11/12/2024.
//

#ifndef GAME_ENGINE_STRUCTURE_H
#define GAME_ENGINE_STRUCTURE_H

#include "Component.h"

#include <glm/glm.hpp>

namespace engine {
  struct Structure {
    enum Type { TYPE_1, TYPE_NONE } type;

    enum Color { COLOR_1, COLOR_2, COLOR_3, COLOR_4, COLOR_MAX} color;

    glm::uvec3 position;

    Structure() : type{TYPE_NONE} {}

    Structure(Type type, Color color, glm::uvec3 position)
      : type{type}, color{color}, position{position} {}

    [[nodiscard]] glm::mat4 mat4() const {
      return glm::mat4 {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {position.x, -float(position.y), position.z, 1}
      };
    }
  };
}

#endif // GAME_ENGINE_STRUCTURE_H
