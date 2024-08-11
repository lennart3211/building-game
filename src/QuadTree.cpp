//
// Created by lennart on 7/5/24.
//

#include "QuadTree.h"

namespace engine {
    void QuadTree::insert( Particle* particle) {
        if (!bounds.contains(particle->position)) return;

        if (particles.size() < capacity || isLeaf()) {
            particles.push_back(particle);
        } else {
            if (isLeaf()) subdivide();

            northWest->insert(particle);
            northEast->insert(particle);
            southWest->insert(particle);
            southEast->insert(particle);
        }
    }

    std::vector< Particle*> QuadTree::query(const Bounds& area)  {
        std::vector< Particle*> found;

        if (!bounds.contains({area.x, area.y}) &&
            !bounds.contains({area.x + area.width, area.y + area.height}))
            return found;

        for ( auto* p : particles) {
            if (area.contains(p->position))
                found.push_back(p);
        }

        if (!isLeaf()) {
            auto nw = northWest->query(area);
            auto ne = northEast->query(area);
            auto sw = southWest->query(area);
            auto se = southEast->query(area);

            found.insert(found.end(), nw.begin(), nw.end());
            found.insert(found.end(), ne.begin(), ne.end());
            found.insert(found.end(), sw.begin(), sw.end());
            found.insert(found.end(), se.begin(), se.end());
        }

        return found;
    }

    void QuadTree::clear() {
        particles.clear();
        northWest.reset();
        northEast.reset();
        southWest.reset();
        southEast.reset();
    }

    void QuadTree::subdivide() {
        float x = bounds.x;
        float y = bounds.y;
        float w = bounds.width / 2;
        float h = bounds.height / 2;

        northWest = std::make_unique<QuadTree>(Bounds{x, y + h, w, h}, capacity);
        northEast = std::make_unique<QuadTree>(Bounds{x + w, y + h, w, h}, capacity);
        southWest = std::make_unique<QuadTree>(Bounds{x, y, w, h}, capacity);
        southEast = std::make_unique<QuadTree>(Bounds{x + w, y, w, h}, capacity);
    }
} // engine