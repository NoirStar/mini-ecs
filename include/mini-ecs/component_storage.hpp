#pragma once

#include <unordered_map>
#include <mini-ecs/entity.hpp>

template<typename T>
class ComponentStorage {
public:
    void add(Entity e, T component) {
        data_.emplace(e, std::move(component));
    }
    
    void remove(Entity e) {
        data_.erase(e);
    }
    
    T* get(Entity e) {
        auto it = data_.find(e);
        if (it != data_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    bool has(Entity e) const {
        return data_.contains(e);
    }

private:
    std::unordered_map<Entity, T> data_;
};