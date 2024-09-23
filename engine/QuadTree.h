#pragma once

#include <memory>
#include "Particle.h"


namespace engine {

    class QuadTree {
    public:
        struct Bounds {
            float x, y, width, height;
            bool contains(const glm::vec2& point) const {
                return point.x >= x && point.x < x + width &&
                       point.y >= y && point.y < y + height;
            }
        };

        explicit QuadTree(const Bounds& bounds, int capacity = 4)
                : bounds(bounds), capacity(capacity) {}

        void insert( Particle *particle);
        [[nodiscard]] std::vector< Particle*> query(const Bounds& area) ;
        void clear();

    private:
        static const int MAX_DEPTH = 10;

        Bounds bounds;
        int capacity;
        std::vector< Particle*> particles;
        std::unique_ptr<QuadTree> northWest;
        std::unique_ptr<QuadTree> northEast;
        std::unique_ptr<QuadTree> southWest;
        std::unique_ptr<QuadTree> southEast;

        void subdivide();
        bool isLeaf() const { return !northWest; }
    };

} // engine
