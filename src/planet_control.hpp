#ifndef PLANET_CONTROL_HPP_INCLUDED
#define PLANET_CONTROL_HPP_INCLUDED

#include "rs4/rs4_sdl.hpp"
#include "world.hpp"

template <class TInput, class TCaster>
class ControlPlanet
{

};

template<class TCaster>
class ControlPlanet<rs4::InputSDL, TCaster>
{
    TCaster * caster;
    rs4::InputSDL * input;
    World * world;
public:
    ControlPlanet(rs4::InputSDL * input, TCaster * c, World * w):
        caster{c}, input{input}, world{w} {SDL_SetRelativeMouseMode(SDL_TRUE);}
    void update();
};

#include "planet_control_impl.hpp"


#endif // PLANET_CONTROL_HPP_INCLUDED
