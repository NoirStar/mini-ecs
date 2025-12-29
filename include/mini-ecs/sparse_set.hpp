#pragma once

#include <mini-ecs/entity.hpp>
#include <vector>
#include <utility>

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

    void remove(Entity entity) {
        uint32_t id = entity & 0xFFFFFFFF;
        if (!has(entity)) return; // 이미 삭제 됨

        size_t removeIdx = sparse_[id];
        Entity lastEntity = entities_.back();
        uint32_t lastId = lastEntity & 0xFFFFFFFF;

        // dense_ 에서 삭제 후 맨 뒤에 있던걸로 채움
        std::swap(dense_[removeIdx], dense_.back());
        std::swap(entities_[removeIdx], entities_.back());

        // sparse 업데이트
        sparse_[lastId] = removeIdx;
        sparse_[id] = -1;

        dense_.pop_back();
        entities_.pop_back();
    }

    T* get(Entity entity) {
        uint32_t id = entity & 0xFFFFFFFF;
        if (!has(entity)) {
            return nullptr;
        }
        return &dense_[sparse_[id]];
    }

    bool has(Entity entity) const {
        uint32_t id = entity & 0xFFFFFFFF;
        return id < sparse_.size() && sparse_[id] != -1;
    }

    auto begin() { return dense_.begin(); }
    auto end() { return dense_.end(); }

private:
    std::vector<size_t> sparse_;
    std::vector<T> dense_;
    std::vector<Entity> entities_;  
};