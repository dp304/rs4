#ifndef SOUND_HPP_INCLUDED
#define SOUND_HPP_INCLUDED


#include "rs4/rs4_sdlgl.hpp"
#include "entt/entt.hpp"
#include "event.hpp"

template<class TAudio>
class SoundMain
{
};

template<>
class SoundMain<rs4::AudioSDL>
{
    rs4::AudioSDL * audio;
public:
    SoundMain(rs4::AudioSDL * audio, entt::DefaultRegistry * registry);
    void update(int dt, std::size_t i1) {}
    template<class TEvent> void onEvent(const TEvent &) {}
};

template<>
void SoundMain<rs4::AudioSDL>::onEvent<EventCollision>(const EventCollision & event);


#endif // SOUND_HPP_INCLUDED
