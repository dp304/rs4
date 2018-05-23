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



struct PlatformSDLGL;
struct VideoSDLGL;

struct PlatformSDLGL
{
    typedef ClockSDL Clock;
    typedef DiskSDL Disk;
    typedef AudioSDL Audio;
    typedef VideoSDLGL Video;
    typedef InputSDL Input;
    Disk * disk;
    Audio * audio;
    Video * video;
    Input * input;
    PlatformSDLGL(const PlatformSDLGL &) = delete;
    PlatformSDLGL(Clock*, Disk * d, Audio * a, Video * v,Input * i):disk{d},audio{a},video{v},input{i}
    {
        if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
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
    struct Mode
    {
        std::string name;
        SDL_DisplayMode displaymode;
    };
    std::vector<Mode> modes;
    int width, height;
    int fullscreen, fullscreen_mode;


    Game * game;


    float aspect;
    bool wireframe = false;

    VideoSDLGL(const VideoSDLGL &) = delete;
    template<class TPlatform>
    VideoSDLGL(TPlatform *, Game * g):
        window{nullptr},
        game{g},
        aspect{1.0f}
    {
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
        {
            if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
        }

        width = game->config.get("resolution_x");
        height = game->config.get("resolution_y");

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0)
            throw std::runtime_error(SDL_GetError());
        if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0)
            throw std::runtime_error(SDL_GetError());
        /*if (SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8) != 0)
            throw std::runtime_error(SDL_GetError());*/


        window = SDL_CreateWindow(
            game->meta.title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
        if (window == nullptr) throw std::runtime_error(SDL_GetError());

        int nmodes = SDL_GetNumDisplayModes(0);
        for (int i = 0; i < nmodes; i++)
        {
            SDL_DisplayMode dm = {0};
            if (SDL_GetDisplayMode(0, i, &dm) != 0)
                throw std::runtime_error(std::string("Failed to get display mode: ")+SDL_GetError());

            // if new resolution
            if (i==0 || dm.w != modes.back().displaymode.w || dm.h != modes.back().displaymode.h )
            {
                dm.refresh_rate = 0;
                dm.driverdata = nullptr;
                modes.push_back(Mode{/*.name =*/ std::to_string(dm.w)+
                                             "x"+
                                             std::to_string(dm.h),
                                    /*.displaymode =*/ dm
                                    }
                                );
            }
        }
        fullscreen = game->config.get("fullscreen");

        fullscreen_mode = game->config.get("fullscreen_mode");
        if ( fullscreen_mode<0 || fullscreen_mode>=(int)(1+modes.size()) )
        {
            fprintf(stderr, "WARNING: Invalid fullscreen mode, setting to 0\n");
            fullscreen_mode=0;
            game->config.set("fullscreen_mode",0);
        }

        game->config.subscribe("fullscreen", [this](const ConfigValue &v)
                                            {
                                                fullscreen = v;
                                                updateFullscreen();
                                            }, false
        );

        game->config.subscribe("fullscreen_mode", [this](const ConfigValue &v)
                                            {
                                                if (v.getI() == fullscreen_mode) return;
                                                fullscreen_mode = v;
                                                if (fullscreen)
                                                    updateFullscreen();
                                            }, false
        );

        updateFullscreen();

        context = SDL_GL_CreateContext(window);
        if (context == nullptr) throw std::runtime_error(SDL_GetError());

        if (SDL_GL_SetSwapInterval(1) != 0) throw std::runtime_error(SDL_GetError());

        glewExperimental = GL_TRUE;
        GLenum glewStatus = glewInit();
        if (glewStatus != GLEW_OK) throw std::runtime_error((char*)glewGetErrorString(glewStatus));

        updateResolution();
    }

    void updateAspect()
    {
        aspect = (float)width/height;
        if (aspect > maxAspect)
        {
            aspect = maxAspect;
            int w1 = std::lround(maxAspect * height);
            glViewport((width-w1)/2, 0,
                       w1, height);
        }
        else if (aspect < minAspect)
        {
            aspect = minAspect;
            int h1 = std::lround(width / minAspect);
            glViewport(0,(height-h1)/2,
                       width, h1);
        }
        else
        {
            glViewport(0,0,width,height);
        }
    }

    void updateResolution()
    {
        SDL_GetWindowSize(window, &width, &height);
        updateAspect();
    }

    void updateFullscreen()
    {
        if (fullscreen==0)
        {
            SDL_SetWindowFullscreen(window, 0);
        }
        else if (fullscreen==1)
        {
            if (fullscreen_mode==0)
            {
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
            else if (fullscreen_mode < (int)(1+modes.size()) )
            {
                SDL_SetWindowDisplayMode(window, &modes[fullscreen_mode-1].displaymode);
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            }
            else
                throw std::runtime_error("Invalid fullscreen mode");
        }
        else
            throw std::runtime_error("Invalid fullscreen value");
        updateResolution();
    }

    static GLuint makeShaderProgram(const GLchar ** vs_src,
                                    GLsizei vs_src_n,
                                    const GLchar ** fs_src,
                                    GLsizei fs_src_n)
    {
        GLint success;
        GLuint vshader, fshader, shader_program;

        vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, vs_src_n, vs_src, nullptr);
        glCompileShader(vshader);
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char log[512];
            glGetShaderInfoLog(vshader, 512, NULL, log);
            throw std::runtime_error(std::string("Vertex shader compilation failed:\n")+log);
        }


        fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, fs_src_n, fs_src, nullptr);
        glCompileShader(fshader);
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char log[512];
            glGetShaderInfoLog(fshader, 512, NULL, log);
            throw std::runtime_error(std::string("Fragment shader compilation failed:\n")+log);
        }

        shader_program = glCreateProgram();
        glAttachShader(shader_program, vshader);
        glAttachShader(shader_program, fshader);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char log[512];
            glGetProgramInfoLog(shader_program, 512, NULL, log);
            throw std::runtime_error(std::string("Shader program linking failed:\n")+log);
        }

        glDeleteShader(vshader);
        glDeleteShader(fshader);

        return shader_program;
    }

    void startRender() {}
    void endRender() { SDL_GL_SwapWindow( window ); }
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
#endif // RS4_SDLGL_HPP_INCLUDED
