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

    rs4::Game * game;
    World * world;

    Control control;
    Physics physics;
    Sound sound;
    Graphics graphics;

public:
    ScreenPlanet(TPlatform * platform, TMachine * m);
    void update(int dt) final
    {
        control.update();
        physics.update(dt);
        sound.update(dt);
    }
    void render(float alpha) final
    {
        graphics.render(alpha);
    }

private:
    void onStart() final {control.start();}
    void onStop() final {}
    void onPause() final {sound.pause();}
    void onUnpause() final {sound.unpause(); control.unpause();}

};


template<class TPlatform, class TMachine>
ScreenPlanet<TPlatform, TMachine>::ScreenPlanet(TPlatform * platform, TMachine * m):
        caster(&sound, m),
        game(m->getGame()),
        world(m->getWorld()),
        control(platform->input, &caster, game, world),
        physics(&caster, game, world),
        sound(platform->audio, game, world),
        graphics(platform->video, game, world)
{
}


#endif // SCREEN_PLANET_HPP_INCLUDED
