#ifndef RENDERER_HPP_INCLUDED
#define RENDERER_HPP_INCLUDED


#include "rs4/rs4_sdlgl.hpp"
#include "entt/entt.hpp"

template <class TVideo>
class RendererMain
{

};

template<>
class RendererMain<rs4::VideoSDLGL>
{
    rs4::VideoSDLGL * video;
    entt::DefaultRegistry * registry;

    GLuint vbo[1];
    GLuint ebo[1];
    GLuint vao[1];
    GLfloat vertexes[24] =
    {
        //  x       y      z    alpha      s      t
         0.5f,   0.5f,  0.0f,    0.5f,  1.0f,  1.0f,
         0.5f,  -0.5f,  0.0f,    1.0f,  1.0f,  0.0f,
        -0.5f,  -0.5f,  0.0f,    0.5f,  0.0f,  0.0f,
        -0.5f,   0.5f,  0.0f,    1.0f,  0.0f,  1.0f
    };
    GLuint indexes[6] =
    {
        0, 1, 3,
        1, 2, 3
    };
    GLuint textures[3];

    GLuint vshader, fshader, shader_program;

    // uniform locations
    GLint u_vertex_colour;
    GLint u_model, u_view, u_projection;


public:
    RendererMain(rs4::VideoSDLGL * video, entt::DefaultRegistry * registry);
    //void update(int dt, std::size_t i1) {}
    void render(float alpha, std::size_t i1);
private:
    static float interpolate(float x0, float x1, float alpha)
    {
        return x0+(x1-x0)*alpha;
        //return x1;
    }
};


// TEST

template<>
class RendererMain<rs4::VideoTest>
{
    rs4::VideoTest * video;
    entt::DefaultRegistry * registry;
public:
    RendererMain(rs4::VideoTest * video, entt::DefaultRegistry * registry):
        video{video}, registry{registry} {}
    //void update(int dt, std::size_t i1) {}
    void render(float alpha, std::size_t i1);
};


#endif // RENDERER_HPP_INCLUDED
