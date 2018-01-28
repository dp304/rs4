#ifndef RS4_SDL_HPP_INCLUDED
#define RS4_SDL_HPP_INCLUDED

#include <vector>
#include <SDL.h>
#include "rs4.hpp"

namespace rs4
{

class PlatformSDL;
class ClockSDL;
class VideoSDL;
class InputSDL;


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

struct VideoSDL
{
    SDL_Window * window;
    SDL_Renderer * renderer;
    VideoSDL(const VideoSDL &) = delete;
    VideoSDL(PlatformSDL *):
        window{nullptr},
        renderer{nullptr}
    {
            if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
            {
                if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());

            }

            window = SDL_CreateWindow(
                "rs4",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                800, 600, SDL_WINDOW_RESIZABLE);
            if (window == nullptr) throw std::runtime_error(SDL_GetError());

            renderer = SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
            if (renderer == nullptr) throw std::runtime_error(SDL_GetError());

    }
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
    }
    void clear()
    {
        std::fill(keyPressed.begin(),keyPressed.end(),false);
        std::fill(mousePressed.begin(),mousePressed.end(),false);
        mouseX = mouseY = 0;
    }
    void processEvent(SDL_Event * event) {
        int button;
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
    typedef VideoSDL Video;
    typedef InputSDL Input;
    InputSDL * input;
    VideoSDL * video;
    PlatformSDL(const PlatformSDL &) = delete;
    PlatformSDL(Clock*,Video* v,Input* i):input{i},video{v}
    {
        if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
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
