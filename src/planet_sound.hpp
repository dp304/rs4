#ifndef PLANET_SOUND_HPP_INCLUDED
#define PLANET_SOUND_HPP_INCLUDED


#include "rs4/rs4_sdlgl.hpp"
#include "world.hpp"
#include "event.hpp"

template<class TAudio>
class SoundPlanet
{
};

template<>
class SoundPlanet<rs4::AudioSDL>
{
    rs4::AudioSDL * audio;
public:
    SoundPlanet(rs4::AudioSDL * audio, World * w);
    void update(int dt, std::size_t i1) {}
    template<class TEvent> void onEvent(const TEvent &) {}
};

template<>
void SoundPlanet<rs4::AudioSDL>::onEvent<EventCollision>(const EventCollision & event);


#endif // PLANET_SOUND_HPP_INCLUDED
