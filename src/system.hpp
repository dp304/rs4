#ifndef SYSTEM_HPP_INCLUDED
#define SYSTEM_HPP_INCLUDED


#include "rs4/rs4.hpp"
#include "entt/entt.hpp"

#include "control.hpp"
#include "physics.hpp"
#include "renderer.hpp"

#include "component.hpp"

template <class TPlatform>
class StateMain
{
    entt::DefaultRegistry registry;

    ControlMain<typename TPlatform::Input> control;
    PhysicsMain physics;
    RendererMain<typename TPlatform::Video> renderer;


public:
    StateMain(rs4::Game * game, TPlatform * platform);
    void update(int dt, std::size_t i1)
    {
        control.update();
        physics.update(dt, i1);
    }
    void render(float alpha, std::size_t i1)
    {
        renderer.render(alpha, i1);
    }
};


template<class TPlatform>
StateMain<TPlatform>::StateMain(rs4::Game * game, TPlatform * platform):
        control(platform->input, &registry),
        physics(&registry, game),
        renderer(platform->video, &registry)
{
    registry.prepare<Position,Velocity,Colour,Health>();
    registry.prepare<Position,Velocity,Colour>();

}



#endif // SYSTEM_HPP_INCLUDED
