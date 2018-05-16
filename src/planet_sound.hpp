#ifndef PLANET_SOUND_HPP_INCLUDED
#define PLANET_SOUND_HPP_INCLUDED


#include "rs4/rs4_sdlglal.hpp"
#include "world.hpp"
#include "component.hpp"
#include "event.hpp"

template<class TAudio>
class SoundPlanet
{
};

template<>
class SoundPlanet<rs4::AudioSDL>
{
    rs4::AudioSDL * audio;

    rs4::Data<rs4::LDSound> uqm_bit;

    bool on = false;
public:
    SoundPlanet(rs4::AudioSDL * audio, rs4::Game * g, World * w);
    void update(int dt) {}
    template<class TEvent> void onEvent(const TEvent &) {}
};

template<>
void SoundPlanet<rs4::AudioSDL>::onEvent<EventCollision>(const EventCollision & event);


// OpenAL


template<>
class SoundPlanet<rs4::AudioAL>
{
    rs4::AudioAL * audio;
    World * world;

    rs4::Data<rs4::LDSound> coll;

    bool on = false;
    ALuint albs[1];
    ALuint alss[1];
public:
    SoundPlanet(rs4::AudioAL * audio, rs4::Game * g, World * w);
    void update(int dt);
    template<class TEvent> void onEvent(const TEvent &) {}
    ~SoundPlanet();
};

template<>
void SoundPlanet<rs4::AudioAL>::onEvent<EventCollision>(const EventCollision & event);


#endif // PLANET_SOUND_HPP_INCLUDED
