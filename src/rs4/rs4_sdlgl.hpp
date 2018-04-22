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
    int width, height;

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

        //cfg_x_resolution = game->config.get("resolution_x");
        //cfg_y_resolution = game->config.get("resolution_y");

        game->config.subscribe("resolution_x", &cfg_x_resolution);
        game->config.subscribe("resolution_y", &cfg_y_resolution);

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
            cfg_x_resolution, cfg_y_resolution, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
        if (window == nullptr) throw std::runtime_error(SDL_GetError());

        context = SDL_GL_CreateContext(window);
        if (context == nullptr) throw std::runtime_error(SDL_GetError());

        if (SDL_GL_SetSwapInterval(1) != 0) throw std::runtime_error(SDL_GetError());

        glewExperimental = GL_TRUE;
        GLenum glewStatus = glewInit();
        if (glewStatus != GLEW_OK) throw std::runtime_error((char*)glewGetErrorString(glewStatus));

        SDL_GetWindowSize(window, &width, &height);
        updateAspect();

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
                video->width = event.window.data1;
                video->height = event.window.data2;
                video->updateAspect();
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
