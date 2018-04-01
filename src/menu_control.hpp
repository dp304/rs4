#ifndef MENU_CONTROL_HPP_INCLUDED
#define MENU_CONTROL_HPP_INCLUDED

#include "rs4/rs4_sdl.hpp"


template <class TInput, class TCaster>
class ControlMenu
{

};

template<class TCaster>
class ControlMenu<rs4::InputSDL, TCaster>
{
    TCaster * caster;
    rs4::InputSDL * input;
    //World * world;
    rs4::IScreen * subscreen = nullptr;
public:
    ControlMenu(rs4::InputSDL * input, TCaster * c):
        caster{c}, input{input} { }
    void update();
    void setSubscreen(rs4::IScreen * s) { subscreen = s; }
};



#include "menu_control_impl.hpp"


#endif // MENU_CONTROL_HPP_INCLUDED
