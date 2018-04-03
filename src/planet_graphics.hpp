#ifndef PLANET_GRAPHICS_HPP_INCLUDED
#define PLANET_GRAPHICS_HPP_INCLUDED


#include "rs4/rs4_sdlgl.hpp"
#include "world.hpp"

template <class TVideo>
class GraphicsPlanet
{

};

template<>
class GraphicsPlanet<rs4::VideoSDLGL>
{
    rs4::VideoSDLGL * video;
    World * world;

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

    GLuint shader_program;

    // uniform locations
    GLint u_vertex_colour;
    GLint u_model, u_view, u_projection;


public:
    GraphicsPlanet(rs4::VideoSDLGL * video, World * w);
    //void update(int dt, std::size_t i1) {}
    void render(float alpha);
private:
    static float interpolate(float x0, float x1, float alpha)
    {
        return x0+(x1-x0)*alpha;
        //return x1;
    }
};


// TEST

template<>
class GraphicsPlanet<rs4::VideoTest>
{
    rs4::VideoTest * video;
    World * world;
public:
    GraphicsPlanet(rs4::VideoTest * video, World * w):
        video{video}, world{w} {}
    //void update(int dt, std::size_t i1) {}
    void render(float alpha);
};


#endif // PLANET_GRAPHICS_HPP_INCLUDED
