#ifndef MENU_GRAPHICS_HPP_INCLUDED
#define MENU_GRAPHICS_HPP_INCLUDED


#include "rs4/rs4_sdlgl.hpp"


template <class TVideo>
class GraphicsMenu
{

};

template<>
class GraphicsMenu<rs4::VideoSDLGL>
{
    rs4::VideoSDLGL * video;
    rs4::IScreen * subscreen = nullptr;

    static const char * vshader_src;
    static const char * fshader_src;

    GLuint shader_program;

    GLuint vbo[1];
    GLuint ebo[1];
    GLuint vao[1];
    GLfloat vertexes[12]= // 4* x y z
    {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    GLuint indexes[6] =
    {
        0, 1, 3,
        1, 2, 3
    };

    GLint u_fade_colour;

    int phase=0;

public:
    GraphicsMenu(rs4::VideoSDLGL * video);
    //void update(int dt) {}
    void start() {phase=0;}
    void update(std::size_t dt);
    void render(float alpha);
    void setSubscreen(rs4::IScreen * s) { subscreen = s; }
};


// TEST

template<>
class GraphicsMenu<rs4::VideoTest>
{
    rs4::VideoTest * video;
public:
    GraphicsMenu(rs4::VideoTest * video):
        video{video} {}
    //void update(int dt, std::size_t i1) {}
    void render(float alpha);
};


#endif // MENU_GRAPHICS_HPP_INCLUDED
