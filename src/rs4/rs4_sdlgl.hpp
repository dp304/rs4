#ifndef RS4_SDLGL_HPP_INCLUDED
#define RS4_SDLGL_HPP_INCLUDED

#include "rs4_sdl.hpp"
#include <GL/glew.h>
#include <SDL_opengl.h>


#define RS4_GLERRORS {\
GLenum err; \
while((err=glGetError())!=GL_NO_ERROR) fprintf(stderr,"%s:%5d\t%X\n",__FILE__,__LINE__,err); \
}

namespace rs4
{

extern const char * vertex_shader_source[];
extern const char * fragment_shader_source[];

extern const int texture_w, texture_h;
extern const unsigned char texture_data[3][64*64*4];



class PlatformSDLGL;
class VideoSDLGL;

struct PlatformSDLGL
{
    typedef ClockSDL Clock;
    typedef AudioSDL Audio;
    typedef VideoSDLGL Video;
    typedef InputSDL Input;
    Audio * audio;
    Video * video;
    Input * input;
    PlatformSDLGL(const PlatformSDLGL &) = delete;
    PlatformSDLGL(Clock*, Audio * a, Video * v,Input * i):audio{a},video{v},input{i}
    {
        if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
        //fprintf(stderr,"%s -- %s\n",SDL_GetPrefPath("rs4","Monster Hunt"), SDL_GetBasePath());
    }
    void handleEvents(Game * game);

    ~PlatformSDLGL()
    {
        SDL_Quit();
    }

};

struct VideoSDLGL
{
    static constexpr float minAspect = 1.0f;
    static constexpr float maxAspect = 16.0f/9.0f;
    SDL_Window * window;
    SDL_GLContext context;

    Game * game;

    int cfg_x_resolution, cfg_y_resolution;


    float aspect;
    bool wireframe = false;

    VideoSDLGL(const VideoSDLGL &) = delete;
    VideoSDLGL(PlatformSDLGL *, Game * g):
        window{nullptr},
        game{g},
        aspect{1.0f}
    {
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
        {
            if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
        }

        cfg_x_resolution = game->config.get("x_resolution", 800);
        cfg_y_resolution = game->config.get("y_resolution", 600);


        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0)
            throw std::runtime_error(SDL_GetError());
        if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0)
            throw std::runtime_error(SDL_GetError());

        window = SDL_CreateWindow(
            "rs4",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            cfg_x_resolution, cfg_y_resolution, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
        if (window == nullptr) throw std::runtime_error(SDL_GetError());

        context = SDL_GL_CreateContext(window);
        if (context == nullptr) throw std::runtime_error(SDL_GetError());

        if (SDL_GL_SetSwapInterval(1) != 0) throw std::runtime_error(SDL_GetError());

        glewExperimental = GL_TRUE;
        GLenum glewStatus = glewInit();
        if (glewStatus != GLEW_OK) throw std::runtime_error((char*)glewGetErrorString(glewStatus));

        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        updateAspect(width, height);

    }

    void updateAspect(int w, int h)
    {
        aspect = (float)w/h;
        if (aspect > maxAspect)
        {
            aspect = maxAspect;
            int w1 = std::lround(maxAspect * h);
            glViewport((w-w1)/2, 0,
                       w1, h);
        }
        else if (aspect < minAspect)
        {
            aspect = minAspect;
            int h1 = std::lround(w / minAspect);
            glViewport(0,(h-h1)/2,
                       w, h1);
        }
        else
        {
            glViewport(0,0,w,h);
        }
    }
    ~VideoSDLGL()
    {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
    }

};

inline void PlatformSDLGL::handleEvents(Game * game)
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
                video->updateAspect(event.window.data1,event.window.data2);
            }
            break;
        }
    }
    if (input->keyPressed[SDL_SCANCODE_F12])
    {
        video->wireframe = !video->wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, (video->wireframe?GL_LINE:GL_FILL));
    }

}


}
#endif // RS4_SDLGL_HPP_INCLUDED
