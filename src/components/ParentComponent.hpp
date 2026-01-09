#pragma once

#include "../ecs/Entity.hpp"

struct ParentComponent {
    Entity parent = NULL_ENTITY;

    ParentComponent() = default;
    ParentComponent(Entity p) : parent(p) {}
};
