#ifndef CONTROL_HPP_INCLUDED
#define CONTROL_HPP_INCLUDED

#include "rs4/rs4_sdl.hpp"
#include "entt/entt.hpp"

template <class TInput>
class ControlMain
{

};

template<>
class ControlMain<rs4::InputSDL>
{
    rs4::InputSDL * input;
    entt::DefaultRegistry * registry;
public:
    ControlMain(rs4::InputSDL * input, entt::DefaultRegistry * registry):
        input{input}, registry{registry} {SDL_SetRelativeMouseMode(SDL_TRUE);}
    void update();
};


template<>
class ControlMain<rs4::InputTest>
{
public:
    ControlMain(rs4::InputTest * input, entt::DefaultRegistry * registry) {}
    void update();
};

#endif // CONTROL_HPP_INCLUDED
