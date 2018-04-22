#ifndef RS4_SDL_HPP_INCLUDED
#define RS4_SDL_HPP_INCLUDED

#include <vector>
#include <SDL.h>
#include "rs4.hpp"

namespace rs4
{

extern const unsigned char pcm_samples[];
extern const int pcm_samples_len;

class PlatformSDL;
class ClockSDL;
class DiskSDL;
class AudioSDL;
class VideoSDL;
class InputSDL;

class StreamSDLFile;

class ClockSDL
{
    unsigned long oldClock;
public:

    template <class TPlatform>
    ClockSDL(TPlatform * platform)
    {
            if (SDL_WasInit(SDL_INIT_TIMER) == 0)
            {
                if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0) throw std::runtime_error(SDL_GetError());

            }

            reset();
    }

    void reset()
    {
        oldClock = SDL_GetTicks();
    }
    void sleep(unsigned long t)
    {
        SDL_Delay(t);
    }

    unsigned long elapsed()
    {
        unsigned long newClock = SDL_GetTicks();
        unsigned long dt = newClock - oldClock;
        oldClock = newClock;
        return dt;
    }

    ~ClockSDL() { }
};

// STREAM

class StreamSDLFile : public IStream
{
public:
    // TODO &&
    StreamSDLFile(const std::string & n, bool b = true):_name{n},_bin{b} {}
private:
    std::string _name;
    bool _bin;
    SDL_RWops * rwops = nullptr;

    void onOpenr() final
    {
        rwops = SDL_RWFromFile(_name.c_str(), _bin ? "rb" : "r");
        if (rwops == nullptr)
            throw std::runtime_error(std::string("Error opening \"") + _name + "\": " + SDL_GetError());

        long long s = SDL_RWsize(rwops);
        if (s==-1)
        {
            const char * err = SDL_GetError();
            if (err[0]!='\0')
                throw std::runtime_error(std::string("Error getting size of \"") + _name + "\": " + err);
        }
        setSize(s);
    }

    void onOpenw() final
    {
        rwops = SDL_RWFromFile(_name.c_str(), _bin ? "wb" : "w");
        if (rwops == nullptr)
            throw std::runtime_error(std::string("Error opening \"") + _name + "\": " + SDL_GetError());
    }

    void onClose() final
    {
        if (SDL_RWclose(rwops) != 0)
            throw std::runtime_error(std::string("Error closing \"") + _name + "\": " + SDL_GetError());
        rwops = nullptr;
    }

    long long onTell() final
    {
        return SDL_RWtell(rwops);
    }

    void onRewind() final
    {
        if (SDL_RWseek(rwops, 0, RW_SEEK_SET) < 0)
        {
            close();
            openr();
        }
    }

    std::size_t onRead(void * buf, std::size_t siz, std::size_t num) final
    {
        std::size_t num1 = SDL_RWread(rwops, buf, siz, num);
        if (num1 == 0)
        {
            const char * err = SDL_GetError();
            if (err[0]!='\0')
                throw std::runtime_error(std::string("Error reading \"") + _name + "\": " + err);
        }
        return num1;
    }
    std::size_t onWrite(const void * buf, std::size_t siz, std::size_t num) final
    {
        std::size_t num1 = SDL_RWwrite(rwops, buf, siz, num);
        if (num1 == 0)
        {
            const char * err = SDL_GetError();
            if (err[0]!='\0')
                throw std::runtime_error(std::string("Error writing \"") + _name + "\": " + err);
        }
        return num1;
    }
};


struct DiskSDL
{
    static constexpr char path_separator =
#ifdef _WIN32
        '\\';
#else
        '/';
#endif // _WIN32

    Game * game;

    std::string pref_path;
    std::string base_path;

    template<class TPlatform>
    DiskSDL(TPlatform *, Game * g):game{g}
    {
        const char * bp = SDL_GetBasePath();
        if (bp == nullptr)
            throw std::runtime_error("Failed to get base path");
        base_path = bp;

        std::string path1, path2;
        for(char c : game->meta.author)
            if (isalnum(c)) path1.push_back(c);
        for(char c : game->meta.title)
            if (isalnum(c)) path2.push_back(c);

        const char * pp = SDL_GetPrefPath(path1.c_str(), path2.c_str());
        if (pp == nullptr)
            pref_path = bp;
        else
            pref_path = pp;

        fprintf(stderr, "Base path: %s\nPref path: %s\n",base_path.c_str(),pref_path.c_str());

        game->config.setStream(new StreamSDLFile(pref_path+path2+".cfg",false));

        try
        {
            game->config.load();
        }
        catch (std::runtime_error & e)
        {
            std::string msg = e.what();
            fprintf(stderr, "WARNING: failed to load configuration file: %s\n", msg.c_str());
            game->config.setDirty();
        }
    }

    ~DiskSDL()
    {
        if (game->config.isDirty())
        {
            try
            {
                game->config.save();
            }
            catch (std::runtime_error & e)
            {
                std::string msg = e.what();
                fprintf(stderr, "WARNING: failed to save configuration file: %s\n", msg.c_str());
            }
        }
    }
};

struct AudioSDL
{
    Game * game;

    SDL_AudioSpec audiospec;
    SDL_AudioDeviceID device;

    Uint8 * pcm_data;
    int pcm_len;
    int pcm_pos;

    template<class TPlatform>
    AudioSDL(TPlatform *, Game * g):
        game{g},
        pcm_data{nullptr},pcm_len{0},pcm_pos{0}
    {
        if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
        {
            if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) throw std::runtime_error(SDL_GetError());
        }

        SDL_AudioSpec audiospec0;
        audiospec0.callback = callback;
        audiospec0.userdata = this;
        audiospec0.channels = 1;
        audiospec0.format = AUDIO_S16;
        audiospec0.freq = 48000;
        audiospec0.samples = 2048;

        if ((device = SDL_OpenAudioDevice(nullptr,0,&audiospec0,&audiospec,0)) == 0)
            throw std::runtime_error(SDL_GetError());

        SDL_PauseAudioDevice(device,1);


    }
    ~AudioSDL()
    {
        SDL_PauseAudioDevice(device,1);
        SDL_CloseAudioDevice(device);
    }
private:
    static void callback(void * userdata, Uint8 * stream, int len)
    {
        AudioSDL * const audio = reinterpret_cast<AudioSDL*>(userdata);
        if (len > audio->pcm_len - audio->pcm_pos)
        {
            const Uint32 len1 = audio->pcm_len - audio->pcm_pos;
            const Uint32 len2 = len - len1;
            SDL_memcpy(stream,        audio->pcm_data + audio->pcm_pos, len1);
            SDL_memset(stream + len1, audio->audiospec.silence,         len2);

            audio->pcm_pos = audio->pcm_len;
        }
        else
        {
            SDL_memcpy(stream, audio->pcm_data + audio->pcm_pos, len);

            audio->pcm_pos += len;
        }
    }
};

struct VideoSDL
{
    SDL_Window * window;
    SDL_Renderer * renderer;

    Game * game;

    int cfg_x_resolution, cfg_y_resolution;

    VideoSDL(const VideoSDL &) = delete;
    VideoSDL(PlatformSDL *, Game * g):
        window{nullptr},
        renderer{nullptr},
        game{g}
    {
            if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
            {
                if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());

            }

            cfg_x_resolution = game->config.get("resolution_x");
            cfg_y_resolution = game->config.get("resolution_y");

            window = SDL_CreateWindow(
                game->meta.title.c_str(),
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                cfg_x_resolution, cfg_y_resolution, SDL_WINDOW_RESIZABLE);
            if (window == nullptr) throw std::runtime_error(SDL_GetError());

            renderer = SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
            if (renderer == nullptr) throw std::runtime_error(SDL_GetError());

    }
    void startRender() {}
    void endRender() { SDL_RenderPresent(renderer); }
    ~VideoSDL()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

};

struct InputSDL
{
    std::vector<bool> keyDown;
    std::vector<bool> keyPressed;
    std::vector<bool> mouseDown;
    std::vector<bool> mousePressed;
    int mouseX, mouseY;

    static const int MAX_UI_EVENTS = 1024;
    SDL_Event ui_events[MAX_UI_EVENTS];
    std::size_t ui_event_n;
    bool ui_enabled;

    template<class TPlatform>
    InputSDL(TPlatform * platform)
    {
        //SDL_SetRelativeMouseMode(SDL_TRUE);
        int siz;
        SDL_GetKeyboardState(&siz);
        keyDown.resize(siz, false);
        keyPressed.resize(siz, false);
        mouseDown.resize(5, false);
        mousePressed.resize(5, false);
        mouseX = mouseY = 0;

        uiOff();
    }
    void clear()
    {
        std::fill(keyPressed.begin(),keyPressed.end(),false);
        std::fill(mousePressed.begin(),mousePressed.end(),false);
        mouseX = mouseY = 0;
    }
    void processEvent(SDL_Event * event) {
        int button;

        if (ui_enabled &&
            ui_event_n < MAX_UI_EVENTS &&
            event->type >= SDL_KEYDOWN &&
            event->type <= SDL_MOUSEWHEEL)
        {
            ui_events[ui_event_n++] = *event;
        }

        switch (event->type)
        {
        case SDL_MOUSEMOTION:
            mouseX += event->motion.xrel;
            mouseY -= event->motion.yrel;
            break;
        case SDL_KEYDOWN:
            if (!event->key.repeat)
            {
                keyPressed[event->key.keysym.scancode]=true;
                keyDown[event->key.keysym.scancode]=true;
            }
            break;
        case SDL_KEYUP:
            keyDown[event->key.keysym.scancode]=false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            button = mapButton(event->button.button);
            mousePressed[button]=true;
            mouseDown[button]=true;
            break;
        case SDL_MOUSEBUTTONUP:
            button = mapButton(event->button.button);
            mouseDown[button]=false;
            break;
        }
    }


    void uiOn() { SDL_SetRelativeMouseMode(SDL_FALSE); ui_enabled = true;  ui_event_n = 0; }
    void uiOff() { SDL_SetRelativeMouseMode(SDL_TRUE); ui_enabled = false; ui_event_n = 0; }
    void uiFlush() { ui_event_n = 0; }

    ~InputSDL()
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }

private:
    static inline int mapButton(int sdlButton) {
        switch (sdlButton)
        {
            case SDL_BUTTON_LEFT:   return 0;
            case SDL_BUTTON_RIGHT:  return 1;
            case SDL_BUTTON_MIDDLE: return 2;
            default:                return 3;
        }
    }
};

struct PlatformSDL
{
    typedef ClockSDL Clock;
    typedef DiskSDL Disk;
    typedef AudioSDL Audio;
    typedef VideoSDL Video;
    typedef InputSDL Input;
    Input * input;
    Disk * disk;
    Audio * audio;
    Video * video;
    PlatformSDL(const PlatformSDL &) = delete;
    PlatformSDL(Clock*,Disk* d,Audio * a, Video * v, Input * i):input{i},disk{d},audio{a},video{v}
    {
        if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
    }
    void handleEvents(Game * game)
    {
        SDL_Event event;

        input->clear();
        while (SDL_PollEvent(&event))
        {
            input->processEvent(&event);
            if (event.type == SDL_QUIT) game->exit();
        }

    }
    ~PlatformSDL()
    {
        SDL_Quit();
    }

};


}


#endif // RS4_SDL_HPP_INCLUDED
