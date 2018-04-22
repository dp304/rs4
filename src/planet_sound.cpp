#include "planet_sound.hpp"


SoundPlanet<rs4::AudioSDL>::SoundPlanet(rs4::AudioSDL * a, rs4::Game * g, World * w)
    :audio{a}
{
    g->config.subscribe("sound",
                    [this](const rs4::ConfigValue & val)
                    {
                        on = (val.getI() > 0);
                        if (!on) SDL_PauseAudioDevice(audio->device, 1);
                    });

    audio->pcm_data = (Uint8*) rs4::pcm_samples;
    audio->pcm_len = audio->pcm_pos = rs4::pcm_samples_len;
    SDL_PauseAudioDevice(audio->device, (on?0:1));
}


template<>
void SoundPlanet<rs4::AudioSDL>::onEvent<EventCollision>(const EventCollision & event)
{
    SDL_PauseAudioDevice(audio->device,1);
    audio->pcm_pos = 0;
    SDL_PauseAudioDevice(audio->device,(on?0:1));
}
