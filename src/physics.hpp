#ifndef PHYSICS_HPP_INCLUDED
#define PHYSICS_HPP_INCLUDED

#include <random>
#include "entt/entt.hpp"
#include "rs4/rs4.hpp"

template <class TEventDispatcher>
class PhysicsMain
{
    TEventDispatcher *events;

    std::random_device rd;
    entt::DefaultRegistry * registry;
    rs4::Game * game;
public:
    PhysicsMain(entt::DefaultRegistry * registry,
                rs4::Game * g,
                TEventDispatcher * e);
    void update(int dt, std::size_t i1);
private:
    static bool collide(float x0, float & x1, float & v, float wall)
    {
        if ((x0<=wall) != (x1<=wall))
        {
            x1 = wall+(wall-x1);
            v = -v;
            return true;
        }
        return false;
    }
};


#include "physics_impl.hpp"

#endif // PHYSICS_HPP_INCLUDED
