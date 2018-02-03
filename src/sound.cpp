#include "sound.hpp"


SoundMain<rs4::AudioSDL>::SoundMain(rs4::AudioSDL * audio, entt::DefaultRegistry * registry)
    :audio{audio}
{
    audio->pcm_data = (Uint8*) rs4::pcm_samples;
    audio->pcm_len = audio->pcm_pos = rs4::pcm_samples_len;
    SDL_PauseAudioDevice(audio->device,0);
}


template<>
void SoundMain<rs4::AudioSDL>::onEvent<EventCollision>(const EventCollision & event)
{
    SDL_PauseAudioDevice(audio->device,1);
    audio->pcm_pos = 0;
    SDL_PauseAudioDevice(audio->device,0);
}
