#ifndef RS4_SDLGLAL_HPP_INCLUDED
#define RS4_SDLGLAL_HPP_INCLUDED

#include "rs4_sdlgl.hpp"
#include <AL/alc.h>
#include <AL/al.h>



namespace rs4
{

class PlatformSDLGLAL;
class AudioAL;

struct PlatformSDLGLAL
{
    typedef ClockSDL Clock;
    typedef DiskSDL Disk;
    typedef AudioAL Audio;
    typedef VideoSDLGL Video;
    typedef InputSDL Input;
    Disk * disk;
    Audio * audio;
    Video * video;
    Input * input;
    PlatformSDLGLAL(const PlatformSDLGLAL &) = delete;
    PlatformSDLGLAL(Clock*, Disk * d, Audio * a, Video * v,Input * i):disk{d},audio{a},video{v},input{i}
    {
        if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
    }
    void handleEvents(Game * game);

    ~PlatformSDLGLAL()
    {
        SDL_Quit();
    }

};

struct AudioAL
{
    Game * game;

    ALCdevice * device = nullptr;
    ALCcontext * context;

    static const char * getError(ALenum e)
    {
        switch (e)
        {
            case AL_NO_ERROR: return "AL_NO_ERROR";
            case AL_INVALID_NAME: return "AL_INVALID_NAME";
            case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
            case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
            case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
            case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
            default: return "AL_?";
        }
    }
    static void handleError(const char * msg)
    {
        ALenum e = alGetError();
        if (e != AL_NO_ERROR)
        throw std::runtime_error(std::string("OpenAL: ") + msg + ": " + getError(e));
    }
    static ALenum getFormat(int channels, int bytes)
    {
        if (channels==1)
        {
            if (bytes==1) return AL_FORMAT_MONO8;
            else if (bytes==2) return AL_FORMAT_MONO16;
            else throw std::runtime_error("Invalid sample size for OpenAL");
        }
        else if (channels==2)
        {
            if (bytes==1) return AL_FORMAT_STEREO8;
            else if (bytes==2) return AL_FORMAT_STEREO16;
            else throw std::runtime_error("Invalid sample size for OpenAL");
        }
        else throw std::runtime_error("Invalid channel count for OpenAL");
    }

    template<class TPlatform>
    AudioAL(TPlatform *, Game * g):
        game{g}
    {
        device = alcOpenDevice(nullptr);
        if (device == nullptr)
            throw std::runtime_error("Failed to open OpenAL device");

        fprintf(stderr, "Loaded OpenAL device \"%s\"\n", alcGetString(device,ALC_DEVICE_SPECIFIER));

        context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);

    }
    ~AudioAL()
    {
        if (context != nullptr)
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
        }
        if (device != nullptr)
            alcCloseDevice(device);
    }
};

inline void PlatformSDLGLAL::handleEvents(Game * game)
{
    SDL_Event event;

    input->clear();

    while (SDL_PollEvent(&event))
    {
        input->processEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            game->exit();
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                /*video->width = event.window.data1;
                video->height = event.window.data2;
                video->updateAspect();*/
                video->updateResolution();
                if (!video->fullscreen)
                {
                    game->config.set("resolution_x",video->width);
                    game->config.set("resolution_y",video->height);
                }
            }
            break;
        }
    }
    if (input->keyPressed[SDL_SCANCODE_F12])
    {
        video->wireframe = !video->wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, (video->wireframe?GL_LINE:GL_FILL));
    }
    //TODO remove
    if (input->keyPressed[SDL_SCANCODE_F2])
    {
        int s = game->config.get("sound");
        game->config.set("sound",!s);
    }


}


}

#endif // RS4_SDLGLAL_HPP_INCLUDED
