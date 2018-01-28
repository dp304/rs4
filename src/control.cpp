#include "rs4/rs4_sdl.hpp"

#include "control.hpp"
#include "component.hpp"

void ControlMain<rs4::InputSDL>::update()
{
    Player &p = registry->get<Player>();
    p.clear();

    p.posX = input->mouseX;
    p.posY = input->mouseY;

    if (input->keyDown[SDL_SCANCODE_W] && !input->keyDown[SDL_SCANCODE_S])
        p.control[p.UP]=true;
    else if (input->keyDown[SDL_SCANCODE_S] && !input->keyDown[SDL_SCANCODE_W])
        p.control[p.DOWN]=true;

    if (input->keyDown[SDL_SCANCODE_A] && !input->keyDown[SDL_SCANCODE_D])
        p.control[p.LEFT]=true;
    else if (input->keyDown[SDL_SCANCODE_D] && !input->keyDown[SDL_SCANCODE_A])
        p.control[p.RIGHT]=true;

    if (input->mousePressed[0])
        p.control[p.FIRE1]=true;
}

// TEST

void ControlMain<rs4::InputTest>::update()
{

}

