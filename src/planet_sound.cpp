#include "planet_sound.hpp"


SoundPlanet<rs4::AudioSDL>::SoundPlanet(rs4::AudioSDL * a, rs4::Game * g, World * w)
    :audio{a},uqm_bit{g}
{
    g->config.subscribe("sound",
                    [this](const rs4::ConfigValue & val)
                    {
                        on = (val.getI() > 0);
                        if (!on) SDL_PauseAudioDevice(audio->device, 1);
                    });

    uqm_bit.load("data/uh.wav");

    //audio->pcm_data = (Uint8*) rs4::pcm_samples;
    //audio->pcm_len = audio->pcm_pos = rs4::pcm_samples_len;
    audio->pcm_data = (Uint8*) &uqm_bit->samples[0];
    audio->pcm_len = audio->pcm_pos = uqm_bit->samples.size();
    SDL_PauseAudioDevice(audio->device, (on?0:1));
}


template<>
void SoundPlanet<rs4::AudioSDL>::onEvent<EventCollision>(const EventCollision & event)
{
    SDL_PauseAudioDevice(audio->device,1);
    audio->pcm_pos = 0;
    SDL_PauseAudioDevice(audio->device,(on?0:1));
}


// OpenAL

SoundPlanet<rs4::AudioAL>::SoundPlanet(rs4::AudioAL * a, rs4::Game * g, World * w)
    :audio{a},world{w},coll{g}
{
    g->config.subscribe("sound",
                    [this](const rs4::ConfigValue & val)
                    {
                        on = (val.getI() > 0);
                        if (!on)  { /*TODO*/ }     // was SDL_PauseAudioDevice(audio->device, 1);
                    });


    alGetError();
    alGenBuffers(1, albs);
    audio->handleError("failed to generate buffers");

    coll.load("data/uh.wav");

    alBufferData(albs[0],
                 audio->getFormat(coll->channels, coll->sample_size),
                 &coll->samples[0], coll->samples.size(),
                 coll->rate);
    audio->handleError("failed to assign buffer");

    alGenSources(1, alss);
    audio->handleError("failed to generate sources");

    alSourcei(alss[0], AL_BUFFER, albs[0]);
    audio->handleError("failed to assign buffer to source");

}


void SoundPlanet<rs4::AudioAL>::update(int dt)
{
    if (!world->registry.has<Camera>())
        return;
    Camera &cam = world->registry.get<Camera>();
    ALfloat listenerPos[]={cam.x, 0.0, cam.distance};
    //ALfloat listenerVel[]={0.0, 0.0, 0.0};
    ALfloat listenerOri[]={0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

    alListenerfv(AL_POSITION,listenerPos);
    //alListenerfv(AL_VELOCITY,listenerVel);
    alListenerfv(AL_ORIENTATION,listenerOri);
}

SoundPlanet<rs4::AudioAL>::~SoundPlanet()
{
    alDeleteSources(1, alss);
    alDeleteBuffers(1, albs);
}

template<>
void SoundPlanet<rs4::AudioAL>::onEvent<EventCollision>(const EventCollision & event)
{
    ALfloat srcPos[] = {event.x, event.y, 0.0};
    alSourcefv(alss[0], AL_POSITION, srcPos);
    alSourcePlay(alss[0]);
}
