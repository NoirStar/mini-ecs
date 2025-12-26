#include "entity.hpp"
#include "component_storage.hpp"
#include "world.hpp"
#include <iostream>

struct Position {
    float x, y;
};

struct Health {
    int current, max;
};

int main() {
    // EntityManager 테스트
    EntityManager em;
    Entity e1 = em.create();
    Entity e2 = em.create();
    
    std::cout << "Entity 1: " << e1 << std::endl;
    std::cout << "Entity 2: " << e2 << std::endl;
    std::cout << "e1 alive: " << em.isAlive(e1) << std::endl;
    
    em.destroy(e1);
    std::cout << "e1 alive after destroy: " << em.isAlive(e1) << std::endl;
    
    Entity e3 = em.create();  // 재사용
    std::cout << "Entity 3: " << e3 << std::endl;
    
    // ComponentStorage 테스트
    ComponentStorage<Position> positions;
    positions.add(e2, {10.0f, 20.0f});
    
    if (Position* pos = positions.get(e2)) {
        std::cout << "e2 position: " << pos->x << ", " << pos->y << std::endl;
    }
    
    // World 테스트 (구현 완료 후)
    // World<Position, Health> world;
    // Entity player = world.createEntity();
    // world.addComponent<Position>(player, {0, 0});
    
    return 0;
}
