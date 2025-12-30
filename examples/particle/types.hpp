#pragma once
#include <mini-ecs/world.hpp>
#include "components.hpp"

using GameWorld = World<Position, Velocity, Lifetime, Particle, SpawnRequest>;