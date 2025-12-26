#pragma once

#include <cstdint>
#include <vector>
#include <queue>

using Entity = uint64_t;

constexpr uint32_t ID_MASK = 0xFFFFFFFF;

class EntityManager {
public:
    EntityManager() = default;
    ~EntityManager() = default;

    Entity create() {
        // 새 슬롯 생성
        if (free_list_.empty()) {
            Entity entity = slots_.size();
            slots_.emplace_back(0, true);
            return entity;
        }

        // 슬롯 재사용
        Entity entity = free_list_.front();
        free_list_.pop();
        
        uint32_t id = entity & ID_MASK;
        uint32_t gen = entity >> 32;
        
        Entity newEntity = (static_cast<uint64_t>(++gen) << 32) | id;
        slots_[id] = {gen, true};
        return newEntity;
    }

    void destroy(Entity entity) {
        uint32_t id = entity & ID_MASK;
        uint32_t gen = entity >> 32;
        if (!isAlive(entity)) {
            return; // 이미 죽음
        }
        slots_[id].alive = false;
        free_list_.push(entity);
    }

    bool isAlive(Entity entity) {
        uint32_t id = entity & ID_MASK;
        uint32_t gen = entity >> 32;
        if (id >= slots_.size()) return false;
        return (gen == slots_[id].generation) && slots_[id].alive;
    }

    struct Slot {
        uint32_t generation = 0;
        bool alive = false;
    };

private:
    std::vector<Slot> slots_;
    std::queue<Entity> free_list_;
};