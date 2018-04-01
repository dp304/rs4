//#include "component.hpp"
#include "event.hpp"


template<class TCaster>
void ControlMenu<rs4::InputSDL, TCaster>::update()
{
    if (input->keyPressed[SDL_SCANCODE_ESCAPE] && subscreen != nullptr)
        caster->signal(EventResume{});
}

// TEST

template<class TCaster>
class ControlMenu<rs4::InputTest, TCaster>
{
public:
    ControlMenu(rs4::InputTest * input, TCaster * c) {}
    void update();
};


template<class TCaster>
void ControlMenu<rs4::InputTest, TCaster>::update()
{

}

