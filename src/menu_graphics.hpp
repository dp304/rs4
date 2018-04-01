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


public:
    GraphicsMenu(rs4::VideoSDLGL * video);
    //void update(int dt) {}
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
