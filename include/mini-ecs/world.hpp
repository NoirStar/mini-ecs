#pragma once

#include <mini-ecs/entity.hpp>
#include <mini-ecs/sparse_set.hpp>
#include <mini-ecs/concepts.hpp>
#include <tuple>
#include <vector>

// ECS의 모든 것을 관리하는 컨테이너
template<typename... Components>
class World {
public:
    World() = default;
    ~World() = default;

    World(World&) = delete;
    World& operator=(World&) = delete;
    World(World&&) = default;
    World& operator=(World&&) = default;

    Entity createEntity() {
        return entityManager_.create();
    }

    void destroyEntity(Entity entity) {
        entityManager_.destroy(entity);
    }

    template<Component... Cs>
    std::vector<Entity> queryEntities() {
        std::vector<Entity> result;
        for (auto e : entityManager_.getAllAlive()) {
            if((hasComponent<Cs>(e) && ...)) {
                result.push_back(e);
            }
        }
        return result;
    }

    bool isAlive(Entity entity) {
        return entityManager_.isAlive(entity);
    }

    template<Component C>
    void addComponent(Entity entity, C component) {
        static_assert(std::is_copy_constructible_v<C>,
            "Component must be copy constructible!");
        static_assert(std::is_default_constructible_v<C>,
            "Component must be default constructible!");
    
        getStorage<C>().add(entity, std::move(component));
    }

    template<Component C>
    void removeComponent(Entity entity) {
        getStorage<C>().remove(entity);
    }

    template<Component C>
    C* getComponent(Entity entity) {
        return getStorage<C>().get(entity);
    }

    template<Component C>
    bool hasComponent(Entity entity) const {
        return getStorage<C>().has(entity);
    }

private:
    EntityManager entityManager_;
    std::tuple<SparseSet<Components>...> storages_;

    template<Component C>
    SparseSet<C>& getStorage() {
        return std::get<SparseSet<C>>(storages_);
    }
    template<Component C>
    const SparseSet<C>& getStorage() const {
        return std::get<SparseSet<C>>(storages_);
    }
};