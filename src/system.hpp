#ifndef SYSTEM_HPP_INCLUDED
#define SYSTEM_HPP_INCLUDED


#include "rs4/rs4.hpp"
#include "entt/entt.hpp"

#include "control.hpp"
#include "physics.hpp"
#include "sound.hpp"
#include "renderer.hpp"

#include "component.hpp"
#include "event.hpp"



template <class TPlatform>
class ScreenMain
{
    class Caster;

    typedef ControlMain<typename TPlatform::Input, Caster> Control;
    typedef PhysicsMain<Caster> Physics;
    typedef SoundMain<typename TPlatform::Audio> Sound;
    typedef RendererMain<typename TPlatform::Video> Renderer;

    struct Caster : rs4::Caster<ScreenMain, Sound>
    {
        using rs4::Caster<ScreenMain, Sound>::Caster;
    };


    Caster events;

    entt::DefaultRegistry registry;

    Control control;
    Physics physics;
    Sound sound;
    Renderer renderer;

public:
    ScreenMain(rs4::Game * game, TPlatform * platform);
    void update(int dt, std::size_t i1)
    {
        control.update();
        physics.update(dt, i1);
    }
    void render(float alpha, std::size_t i1)
    {
        renderer.render(alpha, i1);
    }

    template <class TEvent>
    void onEvent(const TEvent&) {}

    void onEvent(const EventMenu & e) { fprintf(stderr, "MENU!\n"); }
};


template<class TPlatform>
ScreenMain<TPlatform>::ScreenMain(rs4::Game * game, TPlatform * platform):
        events(this, &sound),
        control(platform->input, &registry, &events),
        physics(&registry, game, &events),
        sound(platform->audio, &registry),
        renderer(platform->video, &registry)
{
    registry.prepare<Position,Velocity,Colour,Health>();
    registry.prepare<Position,Velocity,Colour>();

}



#endif // SYSTEM_HPP_INCLUDED
