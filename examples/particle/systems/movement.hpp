#pragma once

#include "system.hpp"
#include "../types.hpp"
#include "../components.hpp"

class MovementSystem : public ISystem {
public:
    MovementSystem() = default;
    ~MovementSystem() = default;

    void update(GameWorld& world, float dt) override {
        std::vector<Entity> entities = world.queryEntities<Position, Velocity>();
        
        for (auto e : entities) {
            Position* ep = world.getComponent<Position>(e);
            Velocity* ev = world.getComponent<Velocity>(e);
            ep->x += ev->vx * dt;
            ep->y += ev->vy * dt;
        }
    }
};