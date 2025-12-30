#pragma once

#include "system.hpp"
#include "../types.hpp"
#include "../components.hpp"

#include <random>

class SpawnSystem : public ISystem {
public:
    SpawnSystem() = default;
    ~SpawnSystem() = default;

    void update(GameWorld& world, float dt) override {
        auto requests = world.queryEntities<SpawnRequest>();
        for (auto e : requests) {
            auto* psr = world.getComponent<SpawnRequest>(e);
            for (int i{}; i<psr->count; ++i) {
                Entity p = world.createEntity();
                world.addComponent<Position>(p, {psr->x, psr->y});
                world.addComponent<Velocity>(p, {velDist_(gen_), velDist_(gen_)});
                world.addComponent<Lifetime>(p, {lifeDist_(gen_)});
                world.addComponent<Particle>(p, {'*'});
            }
        }

        for (auto e : requests) {
            world.destroyEntity(e);
        }
    }

private:
    std::mt19937 gen_{std::random_device{}()};
    std::uniform_real_distribution<float> velDist_{-300.0f, 300.0f};
    std::uniform_real_distribution<float> lifeDist_{1.0f, 3.0f};
};