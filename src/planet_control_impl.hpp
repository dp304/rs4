#include "component.hpp"
#include "event.hpp"

template<class TCaster>
void ControlPlanet<rs4::InputSDL, TCaster>::update()
{
    Player &p = world->registry.get<Player>(world->player_entity);
    p.clear();

    if (input->keyPressed[SDL_SCANCODE_ESCAPE])
        caster->signal(EventMenu{});

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

template<class TCaster>
void ControlPlanet<rs4::InputTest, TCaster>::update()
{

}

