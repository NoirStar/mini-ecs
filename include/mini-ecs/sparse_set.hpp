#pragma once

#include <mini-ecs/entity.hpp>
#include <vector>

// component storage 역할, unordered map 대체
// sparse안에 dense의 위치 기록
template<typename T>
class SparseSet {
public:
    void add(Entity entity, T component) {
        uint32_t id = entity & 0xFFFFFFFF;
        if (id >= sparse_.size()) {
            sparse_.resize(id+1, -1);
        }
        sparse_[id] = dense_.size();
        dense_.push_back(std::move(component));
        entities_.push_back(entity);
    }
    void remove(Entity entity);
    T* get(Entity entity);
    bool has(Entity entity) const {
        
    }

    auto begin() { return dense_.begin(); }
    auto end() { return dense_.end(); }

private:
    std::vector<size_t> sparse_;
    std::vector<T> dense_;
    std::vector<Entity> entities_;  
};