#ifndef WORLD_HPP_INCLUDED
#define WORLD_HPP_INCLUDED

#include "entt/entt.hpp"

struct World
{
    entt::DefaultRegistry registry;
    std::size_t i1 = 0;
    void swapI() { i1 = 1 - i1; }
};

#endif // WORLD_HPP_INCLUDED
