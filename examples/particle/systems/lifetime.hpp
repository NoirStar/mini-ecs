#pragma once

#include "system.hpp"
#include "../types.hpp"
#include "../components.hpp"

class LifetimeSystem : public ISystem {
public:
    LifetimeSystem() = default;
    ~LifetimeSystem() = default;

    void update(GameWorld& world, float dt) override {
        std::vector<Entity> entities = world.queryEntities<Lifetime>();
        std::vector<Entity> toDelete;

        for (auto e : entities) {
            Lifetime* ep = world.getComponent<Lifetime>(e);
            ep->remaining -= dt;
            if (ep->remaining <= 0) {
                toDelete.push_back(e);
            }
        }

        for (Entity e : toDelete) {
            world.destroyEntity(e);
        }
    }
};