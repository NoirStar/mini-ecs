#include <iostream>
#include <format>
#include <thread>
#include <chrono>

#include <mini-ecs/world.hpp>
#include "types.hpp"
#include "components.hpp"

using namespace std::chrono_literals;

int main() {
    // 1. World 생성 (컴포넌트 타입들 등록)
    GameWorld world;

    Entity particle = world.createEntity();

    // particle이라는 Entity에 Component를 추가.
    world.addComponent<Position>(particle, {40.0f, 12.0f});
    world.addComponent<Velocity>(particle, {1.0f, 0.0f});
    world.addComponent<Lifetime>(particle, {5.0f});
    world.addComponent<Particle>(particle, {'*'});
    
    for (int i{}; i<10; ++i) {
        // 1. 입력 (마우스클릭 위치에 터지기)
        
        // 2. 업데이트
        std::vector<Entity> entities = world.queryEntities<Position, Velocity>();
        for (auto& e : entities) {
            Position* ep = world.getComponent<Position>(e);
            Velocity* ev = world.getComponent<Velocity>(e);
            ep->x += ev->vx;
            ep->y += ev->vy;
        }
        
        // 4. 대기
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}