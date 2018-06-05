#ifndef RS4_SDLGLAL_HPP_INCLUDED
#define RS4_SDLGLAL_HPP_INCLUDED

#include "rs4_sdlgl.hpp"
#include <AL/alc.h>
#include <AL/al.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <exception>



namespace rs4
{

struct PlatformSDLGLAL;
struct AudioAL;

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

class StreamMusic : public IStream
{
protected:
    std::unique_ptr<IStream> source_stream;
    bool loop = false;
    int channels = -1;
    int rate = -1;
    int sample_size = -1;
public:
    StreamMusic(std::unique_ptr<IStream> && src):source_stream{std::move(src)} {}
    bool getLoop() const { return loop; }
    void setLoop(bool l) { loop = l; }
    int getChannels() const { return channels; }
    int getRate() const { return rate; }
    int getSampleSize() const { return sample_size; }
    int getBPS() const
    {
        if (channels<0||rate<0||sample_size<0) return -1;
        return channels*rate*sample_size;
    }
};

struct AudioAL
{
    //typedef PlatformSDLGLAL::Clock Clock;
    Game * game;

    ALCdevice * device = nullptr;
    ALCcontext * context = nullptr;

    bool sound_on, music_on;
    float gain_master, gain_music, gain_sound;

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
        static constexpr ALenum fmts[4] = {AL_FORMAT_MONO8,
                                           AL_FORMAT_MONO16,
                                           AL_FORMAT_STEREO8,
                                           AL_FORMAT_STEREO16};
        if (bytes<0 || bytes>2)
            throw std::runtime_error("Invalid sample size for OpenAL");
        if (channels<0 || channels>2)
            throw std::runtime_error("Invalid channel count for OpenAL");

        return fmts[2*(channels-1)+(bytes-1)];
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

        music_bufs.resize(4);
        alGetError();
        alGenBuffers(music_bufs.size(), &music_bufs[0]);
        handleError("failed to generate music buffers");
        alGenSources(1, &music_source);
        handleError("failed to generate music source");

        game->config.subscribe("sound",
                [this](const ConfigValue & val)
                {
                    sound_on = (val.getI()!=0);
                }
         );
        game->config.subscribe("music",
                [this](const ConfigValue & val)
                {
                    music_on = (val.getI()!=0);
                    if (!music_on)
                        stopMusic();
                }
         );
        game->config.subscribe("volume_master",
                [this](const ConfigValue & val)
                {
                    gain_master = val.getI()/100.0f;
                    alListenerf(AL_GAIN, gain_master);
                }
         );
        game->config.subscribe("volume_music",
                [this](const ConfigValue & val)
                {
                    gain_music = val.getI()/100.0f;
                    alSourcef(music_source, AL_GAIN, gain_music);
                }
         );
        game->config.subscribe("volume_sound",
                [this](const ConfigValue & val)
                {
                    gain_sound = val.getI()/100.0f;
                    //alListenerf(AL_GAIN, gain_sound);
                }
         );

        thr_music = std::thread(&AudioAL::runMusicThread, this, 200);

    }
    ~AudioAL()
    {
        try
        {
            killMusicThread();
        }
        catch (std::exception & e)
        {
            fprintf(stderr, "ERROR in music thread: %s\n", e.what());
        }
        if (thr_music.joinable())
            thr_music.join();

        alDeleteSources(1, &music_source);
        alDeleteBuffers(music_bufs.size(), &music_bufs[0]);

        if (context != nullptr)
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
        }
        if (device != nullptr)
            alcCloseDevice(device);
    }

    // MUSIC

    std::thread thr_music;

    // mutex etc.
    std::mutex music_mtx;
    std::condition_variable music_cv;
    std::exception_ptr thr_music_eptr;
    enum music_command_t {MC_READY, MC_ERROR, MC_PLAY, MC_PAUSE, MC_RESUME, MC_STOP, MC_KILL};
    music_command_t music_command = MC_READY;

    void playMusic()
    {
        sendMusicCommand(MC_PLAY, nullptr);
    }

    void playMusic(std::unique_ptr<StreamMusic> && stream)
    {
        sendMusicCommand(MC_PLAY, std::move(stream));
    }

    void stopMusic()
    {
        sendMusicCommand(MC_STOP, nullptr);
    }

    void pauseMusic()
    {
        sendMusicCommand(MC_PAUSE, nullptr);
    }

    void resumeMusic()
    {
        sendMusicCommand(MC_RESUME, nullptr);
    }

    bool isMusicPaused() const
    {
        ALint source_state;
        alGetSourcei(music_source, AL_SOURCE_STATE, &source_state);
        return source_state == AL_PAUSED;
    }

private:

    ALuint music_source;
    std::vector<ALuint> music_bufs;

    std::unique_ptr<rs4::StreamMusic> next_stream;


    void runMusicThread(int interval)
    {
        thr_music_eptr = nullptr;
        try
        {
            enum {ST_IDLE, ST_STARTING, ST_STREAMING, ST_STOPPING} state = ST_IDLE;
            ALint source_state;

            std::vector<ALuint> bufs_to_fill(music_bufs.size());
            ALint num_bufs_to_fill;
            std::vector<char> sampleBuf;

            std::unique_ptr<StreamMusic> str, str_next;

            while (true)
            {
                fprintf(stderr, "MUSIC THREAD\n");

                // process commands

                {
                    //alSourcei()

                    std::cv_status cvs = std::cv_status::no_timeout;
                    std::unique_lock<std::mutex> ulk(music_mtx);
                    if (state == ST_IDLE)
                    {
                        while (music_command==MC_READY)
                            music_cv.wait(ulk);
                    }
                    else
                    {
                        while (cvs == std::cv_status::no_timeout && music_command==MC_READY)
                            cvs = music_cv.wait_for(ulk, std::chrono::milliseconds{interval});
                    }

                    if (music_command == MC_KILL)
                    {
                        fprintf(stderr, "MUSIC KILL\n");
                        break;
                    }

                    switch (music_command)
                    {
                    case MC_READY:
                        break;
                    case MC_PLAY:
                        fprintf(stderr, "MUSIC PLAY\n");
                        if (next_stream)
                            str_next = std::move(next_stream);
                        state = ST_STARTING;
                        break;
                    case MC_PAUSE:
                        fprintf(stderr, "MUSIC PAUSE\n");
                        alGetSourcei(music_source, AL_SOURCE_STATE, &source_state);
                        if (source_state == AL_PLAYING)
                        {
                            alSourcePause(music_source);
                            state = ST_IDLE;
                        }
                        break;
                    case MC_RESUME:
                        fprintf(stderr, "MUSIC RESUME\n");
                        alGetSourcei(music_source, AL_SOURCE_STATE, &source_state);
                        if (source_state == AL_PAUSED)
                        {
                            alSourcePlay(music_source);
                            state = ST_STREAMING;
                        }
                        break;
                    case MC_STOP:
                        fprintf(stderr, "MUSIC STOP\n");
                        state = ST_STOPPING;
                        break;
                    default:
                        break;
                    };

                    music_command = MC_READY;
                }
                music_cv.notify_one();

                // prepare buffers

                if (state == ST_STARTING)
                {
                    alSourceStop(music_source);
                    handleError("stopping music");
                    alSourcei(music_source, AL_BUFFER, 0);
                    handleError("removing music source buffers");
                    if (str && str->isOpen()) str->close();

                    if (str_next)
                    {
                        str = std::move(str_next);
                    }

                    if (str)
                    {
                        str->openr();
                        int siz_unit = str->getChannels()*str->getSampleSize();
                        int siz = str->getBPS();
                        if (siz<0)
                            throw std::runtime_error("Failed to get music bytes per second");
                        siz = siz*interval/1000;
                        siz = (siz/siz_unit + 1)*siz_unit;
                        sampleBuf.resize(siz);
                        std::copy(music_bufs.begin(), music_bufs.end(), bufs_to_fill.begin());
                        num_bufs_to_fill = (ALint)music_bufs.size();
                        state = ST_STREAMING;
                    }
                    else
                    {
                        state = ST_IDLE;
                    }
                }
                else if (state == ST_STREAMING)
                {
                    alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &num_bufs_to_fill);
                    handleError("getting processed music buffers");
                    alSourceUnqueueBuffers(music_source, num_bufs_to_fill, &bufs_to_fill[0]);
                    handleError("unqueueing music buffers");
                }

                // stream

                if (state == ST_STREAMING)
                {
                    std::size_t num_filled = 0;
                    bool more = true;
                    while (more && (ALint)num_filled < num_bufs_to_fill)
                    {
                        std::size_t n = str->read(&sampleBuf[0], 1, sampleBuf.size());
                        fprintf(stderr,"F%lu \tA%lu \tB%lu\n",num_filled,n,sampleBuf.size());
                        if (n < sampleBuf.size())
                        {
                            more = false;
                        }
                        if (n > 0)
                        {
                            alBufferData(bufs_to_fill[num_filled],
                                         getFormat(str->getChannels(), str->getSampleSize()),
                                         &sampleBuf[0], n,
                                         str->getRate()
                                         );
                            handleError("setting music buffer data");
                            num_filled++;
                        }
                    }
                    alSourceQueueBuffers(music_source, num_filled, &bufs_to_fill[0]);
                    handleError("queueing music buffers");

                    alGetSourcei(music_source, AL_SOURCE_STATE, &source_state);
                    if (num_filled > 0 && source_state != AL_PLAYING)
                    {
                        fprintf(stderr,"was not playing!\n");
                        alSourcePlay(music_source);
                    }
                    if (num_filled == 0 && source_state == AL_STOPPED)
                    {
                        state = ST_STOPPING;
                    }
                }

                // stop music

                if (state == ST_STOPPING)
                {
                    alSourceStop(music_source);
                    handleError("stopping music");
                    alSourcei(music_source, AL_BUFFER, 0);
                    handleError("removing music source buffers");
                    if (str && str->isOpen()) str->close();
                    state = ST_IDLE;
                }
            }

            {
                std::lock_guard<std::mutex> glk(music_mtx);
                music_command = MC_ERROR;
            }
            music_cv.notify_all();
        }
        catch (std::exception &e)
        {
            {
                std::lock_guard<std::mutex> glk(music_mtx);
                thr_music_eptr = std::current_exception();
                music_command = MC_ERROR;
                fprintf(stderr, "MUSIC ERROR: %s\n", e.what());
            }
            music_cv.notify_all();
        }
    }
    void killMusicThread()
    {
        {
            std::unique_lock<std::mutex> ulk(music_mtx);
            while (music_command!=MC_READY && music_command!=MC_ERROR)
                music_cv.wait(ulk);
            handleMusicError();
            music_command = MC_KILL;
        }
        music_cv.notify_all();
    }

    void handleMusicError()
    {
        if (music_command!=MC_ERROR)
            return;
        if (thr_music_eptr)
            std::rethrow_exception(thr_music_eptr);
        else
            throw std::runtime_error("Music thread exited \"normally\"");
    }

    void sendMusicCommand(music_command_t cmd, std::unique_ptr<StreamMusic> && str)
    {
        {
            std::unique_lock<std::mutex> ulk(music_mtx);
            while (music_command!=MC_READY && music_command!=MC_ERROR)
                music_cv.wait(ulk);
            handleMusicError();
            if (str) next_stream = std::move(str);
            music_command = cmd;
        }
        music_cv.notify_all();
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


std::unique_ptr<StreamMusic> makeStreamMusicVorbis(std::unique_ptr<IStream> && stream);

}

#endif // RS4_SDLGLAL_HPP_INCLUDED
