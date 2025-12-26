#pragma once

#include "entity.hpp"
#include "component_storage.hpp"
#include <tuple>

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
    bool isAlive(Entity entity) {
        return entityManager_.isAlive(entity);
    }

    template<typename C>
    void addComponent(Entity entity, C component) {
        getStorage<C>().add(entity, std::move(component));
    }

    template<typename C>
    void removeComponent(Entity entity) {
        getStorage<C>().remove(entity);
    }

    template<typename C>
    C* getComponent(Entity entity) {
        return getStorage<C>().get(entity);
    }

    template<typename C>
    bool hasComponent(Entity entity) const {
        return getStorage<C>().has(entity);
    }

private:
    EntityManager entityManager_;
    std::tuple<ComponentStorage<Components>...> storages_;

    template<typename C>
    ComponentStorage<C>& getStorage() {
        return std::get<ComponentStorage<C>>(storages_);
    }
    template<typename C>
    const ComponentStorage<C>& getStorage() const {
        return std::get<ComponentStorage<C>>(storages_);
    }
};