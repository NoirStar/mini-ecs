#pragma once

#include "system.hpp"
#include "../types.hpp"
#include "../components.hpp"
#include <array>
#include <iostream>
#include <cstdlib>

constexpr int WIDTH = 80;
constexpr int HEIGHT = 24;

class RenderSystem : public ISystem {
public:
    RenderSystem() = default;
    ~RenderSystem() = default;

    void update(GameWorld& world, float dt) override {
        std::vector<Entity> entities = world.queryEntities<Position, Particle>();

        for (auto e : entities) {
            Particle* par = world.getComponent<Particle>(e);
            Position* pos = world.getComponent<Position>(e);

            for (size_t i{}; i<WIDTH; ++i) {
                for (size_t j{}; j<HEIGHT; ++j) {
                    
                }
            }
        }
    }

private:
    std::array<std::array<char, WIDTH>, HEIGHT> buffer_{};
};