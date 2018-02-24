#ifndef CONTROL_HPP_INCLUDED
#define CONTROL_HPP_INCLUDED

#include "rs4/rs4_sdl.hpp"
#include "entt/entt.hpp"

template <class TInput, class TCaster>
class ControlMain
{

};

template<class TCaster>
class ControlMain<rs4::InputSDL, TCaster>
{
    TCaster * events;
    rs4::InputSDL * input;
    entt::DefaultRegistry * registry;
public:
    ControlMain(rs4::InputSDL * input, entt::DefaultRegistry * registry, TCaster *e):
        events{e},input{input}, registry{registry} {SDL_SetRelativeMouseMode(SDL_TRUE);}
    void update();
};

#include "control_impl.hpp"


#endif // CONTROL_HPP_INCLUDED
