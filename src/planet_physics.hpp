#ifndef PLANET_PHYSICS_HPP_INCLUDED
#define PLANET_PHYSICS_HPP_INCLUDED

#include <random>
#include "world.hpp"

template <class TCaster>
class PhysicsPlanet
{
    TCaster * caster;

    std::random_device rd;
    World * world;
public:
    PhysicsPlanet(TCaster * c, rs4::Game * g, World * w);
    void update(int dt);
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


#include "planet_physics_impl.hpp"

#endif // PLANET_PHYSICS_HPP_INCLUDED
