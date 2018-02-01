#ifndef SYSTEM_HPP_INCLUDED
#define SYSTEM_HPP_INCLUDED


#include "rs4/rs4.hpp"
#include "entt/entt.hpp"

#include "control.hpp"
#include "physics.hpp"
#include "renderer.hpp"

#include "component.hpp"
#include "event.hpp"

class SoundMain
{
public:
    void update(int dt, std::size_t i1) {}
    template<class TEvent> void onEvent(const TEvent &) {}
};

template<>
inline void SoundMain::onEvent<EventCollision>(const EventCollision & event)
{
    fprintf(stderr, "BUMM!\n");
}



template <class TPlatform>
class ScreenMain
{
    class EventDispatcher;

    typedef ControlMain<typename TPlatform::Input> Control;
    typedef PhysicsMain<EventDispatcher> Physics;
    typedef SoundMain Sound;
    typedef RendererMain<typename TPlatform::Video> Renderer;

    struct EventDispatcher : rs4::EventDispatcher<ScreenMain, Sound>
    {
        using rs4::EventDispatcher<ScreenMain, Sound>::EventDispatcher;
    };


    EventDispatcher events;

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
};


template<class TPlatform>
ScreenMain<TPlatform>::ScreenMain(rs4::Game * game, TPlatform * platform):
        events(this, &sound),
        control(platform->input, &registry),
        physics(&registry, game, &events),
        renderer(platform->video, &registry)
{
    registry.prepare<Position,Velocity,Colour,Health>();
    registry.prepare<Position,Velocity,Colour>();

}



#endif // SYSTEM_HPP_INCLUDED
