//#include "component.hpp"
#include "event.hpp"

template<class TCaster>
void ControlMenu<rs4::InputSDL, TCaster>::update()
{
    if (input->keyPressed[SDL_SCANCODE_ESCAPE] && subscreen != nullptr)
        caster->signal(EventResume{});

    ui->beginInput();
    for (std::size_t i = 0; i<input->ui_event_n; i++)
    {
        nk_sdl_handle_event(&input->ui_events[i]);
    }
    input->uiFlush();
    ui->endInput();

}

// TEST

template<class TCaster>
void ControlMenu<rs4::InputTest, TCaster>::update()
{

}

