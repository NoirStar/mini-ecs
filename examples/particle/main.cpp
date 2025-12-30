#include <iostream>
#include <format>
#include <thread>
#include <chrono>
#include <random>

#include <mini-ecs/world.hpp>
#include "types.hpp"
#include "components.hpp"
#include "systems/movement.hpp"
#include "systems/lifetime.hpp"
#include "systems/render.hpp"
#include "systems/input.hpp"
#include "systems/spawn.hpp"

using namespace std::chrono_literals;

int main() {
    // 1. World 생성 (컴포넌트 타입들 등록)
    GameWorld world;

    // System 생성
    MovementSystem movement;
    LifetimeSystem lifetime;
    RenderSystem render;
    InputSystem input;
    SpawnSystem spawn;

    while (true) {
        float dt = 0.016f;  // 약 60 FPS
        input.update(world, dt);
        spawn.update(world, dt);
        movement.update(world, dt);
        lifetime.update(world, dt);
        render.update(world, dt);
        
        std::this_thread::sleep_for(16ms);
    }

    return 0;
}