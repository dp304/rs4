#ifndef PLANETSTORM18_HPP_INCLUDED
#define PLANETSTORM18_HPP_INCLUDED

#include <typeinfo>

#include "rs4/rs4.hpp"
#include "world.hpp"

#include "event.hpp"

#include "screen_planet.hpp"


template<class TPlatform>
class PlanetStorm18
{
    World world;

    rs4::Game * game;

    //ScreenMenu<TPlatform> screen_menu;
    ScreenPlanet<TPlatform, PlanetStorm18> screen_planet;

    //...



    rs4::IScreen * currentScreen = &screen_planet;

    typedef std::function<rs4::IScreen*(rs4::IScreen*)> transition_fn_t;
    int transition_priority = 0;
    transition_fn_t transition_fn;

    void transition(int priority, transition_fn_t t)
    {
        if (priority < transition_priority)
            return;
        transition_fn = t;
        transition_priority = priority;
    }

public:
    PlanetStorm18(rs4::Game * g, TPlatform * p):
        game{g},
        //screen_menu(g,p,this),
        screen_planet(p,this,&world) {}

    void update(int dt, std::size_t i1)
    {
        while (transition_priority > 0)
        {
            transition_priority = 0;
            currentScreen = transition_fn(currentScreen);
            fprintf(stderr, "New state: \"%s\"\n", typeid(*currentScreen).name());
        }

        //if (!currentScreen->isPaused())
        {
            currentScreen->update(dt, i1);
        }
    }

    void render(float alpha, std::size_t i1)
    {
        currentScreen->render(alpha, i1);
    }

    template <class TEvent>
    void onEvent(const TEvent&) {}

    void onEvent(const EventMenu & e) { fprintf(stderr, "MENU!\n"); }

    void onEvent(const EventGameOver & e) { game->exit(); }

};


#endif // PLANETSTORM18_HPP_INCLUDED
