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

using namespace std::chrono_literals;

int main() {
    // 1. World 생성 (컴포넌트 타입들 등록)
    GameWorld world;

    // 랜덤 엔진
    std::random_device rd;
    std::mt19937 gen(rd());

    // 분포 범위 (속도, 라이프타임)
    std::uniform_real_distribution<float> velDist(-100.0f, 100.0f);
    std::uniform_real_distribution<float> lifeDist(1.0f, 3.0f);

    for (int i{}; i<1000; i++) {
        Entity p = world.createEntity();
        world.addComponent<Position>(p, {40.0f, 12.0f});  // 중앙
        world.addComponent<Velocity>(p, {velDist(gen), velDist(gen)});
        world.addComponent<Lifetime>(p, {lifeDist(gen)});
        world.addComponent<Particle>(p, {'*'});
    }

    // System 생성
    MovementSystem movement;
    LifetimeSystem lifetime;
    RenderSystem render;

    while (true) {
        float dt = 0.016f;  // 약 60 FPS
        movement.update(world, dt);
        lifetime.update(world, dt);
        render.update(world, dt);
        
        if (world.queryEntities<Particle>().empty()) {
            break;
        }
        std::this_thread::sleep_for(16ms);
    }

    return 0;
}