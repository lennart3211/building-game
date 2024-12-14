//
// Created by mclen on 11/12/2024.
//

#ifndef GAME_ENGINE_GAME_H
#define GAME_ENGINE_GAME_H

#include "Grid3D.h"
#include "Structure.h"

#include "glm/ext/vector_uint3.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace engine {

template <uint32_t WIDTH, uint32_t HEIGHT, uint32_t DEPTH>
class Game {
  std::vector<std::optional<Structure>> m_structures{WIDTH * HEIGHT * DEPTH};

public:

  struct RayHitResult {
    glm::uvec3 structurePos;
    glm::uvec3 adjacentPos;
    bool hit = false;
  };

  void PlaceStructure(glm::uvec3 location, Structure::Color color) {
    if (!isValidPosition(location)) {
      return;
    }

    uint32_t index = getFlatIndex(location);
    m_structures[index] = Structure(Structure::TYPE_1, color, location);
  }

  void ChangeColor(glm::uvec3 location, Structure::Color color) {
    uint32_t index = getFlatIndex(location);

    if (index >= m_structures.size()) {
      return;
    }

    if (m_structures[index].has_value()) {
      m_structures[index]->color = color;
    }
  }

  [[nodiscard]] bool isValidPosition(glm::uvec3 pos) const {
    return pos.x < WIDTH && pos.y < HEIGHT && pos.z < DEPTH;
  }

  [[nodiscard]] const std::optional<Structure>& getStructure(glm::uvec3 pos) const {
    if (!isValidPosition(pos)) {
      static const std::optional<Structure> empty;
      return empty;
    }
    return m_structures[getFlatIndex(pos)];
  }

  [[nodiscard]] const std::vector<std::optional<Structure>> &GetStructures() const { return m_structures; }

  [[nodiscard]] RayHitResult intersectsStructure(glm::vec3 origin, glm::vec3 direction) const {
    origin.y = -origin.y; // vulkan to normal coords
    direction.y = -direction.y;
    direction = glm::normalize(direction);

    float closestDist = std::numeric_limits<float>::max();
    RayHitResult result;

    for (uint32_t y = 0; y < HEIGHT; y++) {
      for (uint32_t z = 0; z < DEPTH; z++) {
        for (uint32_t x = 0; x < WIDTH; x++) {
          const auto& structure = getStructure({x, y, z});
          if (!structure.has_value()) continue;

          glm::vec3 boxMin(x - 0.5, y - 0.5, z - 0.5);
          glm::vec3 boxMax(x + 0.5, y + 0.5, z + 0.5);

          glm::vec3 invDir = 1.0f / direction;
          glm::vec3 t0 = (boxMin - origin) * invDir;
          glm::vec3 t1 = (boxMax - origin) * invDir;

          glm::vec3 tmin = glm::min(t0, t1);
          glm::vec3 tmax = glm::max(t0, t1);

          float enterDist = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
          float exitDist = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

          if (enterDist <= exitDist && enterDist > 0 && enterDist < closestDist) {
            closestDist = enterDist;
            result.hit = true;
            result.structurePos = glm::uvec3(x, y, z);

            if (enterDist == tmin.x) {
              result.adjacentPos = direction.x > 0 ? result.structurePos + glm::uvec3{-1, 0, 0} : result.structurePos + glm::uvec3{1, 0, 0};
            }
            else if (enterDist == tmin.y) {
              result.adjacentPos = direction.y > 0 ? result.structurePos + glm::uvec3{0, -1, 0} : result.structurePos + glm::uvec3{0, 1, 0};
            }
            else {
              result.adjacentPos = direction.z > 0 ? result.structurePos + glm::uvec3{0, 0, -1} : result.structurePos + glm::uvec3{0, 0, 1};
            }
          }
        }
      }
    }

    return result;
  }
private:
//  Structure::Type descideType(uint32_t index) {
//    return Structure::TYPE_1;
//  }

  [[nodiscard]] uint32_t getFlatIndex(glm::uvec3 pos) const {
    return pos.x + pos.z * WIDTH + pos.y * WIDTH * DEPTH;
  }

};

} // namespace engine

#endif // GAME_ENGINE_GAME_H
