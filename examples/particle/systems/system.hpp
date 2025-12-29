#pragma once
#include "../types.hpp"

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(GameWorld& world, float dt) = 0;
};