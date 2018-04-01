#ifndef SCREEN_PLANET_HPP_INCLUDED
#define SCREEN_PLANET_HPP_INCLUDED

#include "world.hpp"

#include "planet_control.hpp"
#include "planet_physics.hpp"
#include "planet_sound.hpp"
#include "planet_graphics.hpp"


template <class TPlatform, class TMachine>
class ScreenPlanet: public rs4::IScreen
{
    class Caster;

    typedef ControlPlanet<typename TPlatform::Input, Caster> Control;
    typedef PhysicsPlanet<Caster> Physics;
    typedef SoundPlanet<typename TPlatform::Audio> Sound;
    typedef GraphicsPlanet<typename TPlatform::Video> Graphics;

    struct Caster : rs4::Caster<Sound, TMachine>
    {
        using rs4::Caster<Sound, TMachine>::Caster;
    };


    Caster caster;

    World * world;

    Control control;
    Physics physics;
    Sound sound;
    Graphics graphics;

public:
    ScreenPlanet(TPlatform * platform, TMachine * m, World * w);
    void update(int dt) final
    {
        control.update();
        physics.update(dt);
    }
    void render(float alpha) final
    {
        graphics.render(alpha);
    }

private:
    void onStart() final {}
};


template<class TPlatform, class TMachine>
ScreenPlanet<TPlatform, TMachine>::ScreenPlanet(TPlatform * platform, TMachine * m, World * w):
        caster(&sound, m),
        world(w),
        control(platform->input, &caster, w),
        physics(&caster, w),
        sound(platform->audio, w),
        graphics(platform->video, w)
{
    world->registry.prepare<Position,Velocity,Colour,Health>();
    world->registry.prepare<Position,Velocity,Colour>();
}


#endif // SCREEN_PLANET_HPP_INCLUDED
