#ifndef PLANETSTORM18_HPP_INCLUDED
#define PLANETSTORM18_HPP_INCLUDED

#include <typeinfo>

#include "rs4/rs4.hpp"
#include "world.hpp"

#include "event.hpp"

#include "screen_planet.hpp"
#include "screen_menu.hpp"


template<class TPlatform>
class PlanetStorm18
{

    typename TPlatform::Video * video;
    rs4::Game * game;

    World world;

    ScreenMenu<TPlatform, PlanetStorm18> screen_menu;
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
        video{p->video},
        game{g},
        screen_menu(p,this),
        screen_planet(p,this)
        {
            // FOR TESTING
            rs4::Data<rs4::LDText> txt1(g), txt2(g), txt3(g);
            fprintf(stderr, "load txt1:\n");
            txt1.load("data/x.txt");
            fprintf(stderr, "load txt2:\n");
            txt2.load("data/x.txt");
            fprintf(stderr, "load txt3:\n");
            txt3.load("data/x.txt");
            fprintf(stderr, "unload txt2:\n");
            txt2.unload();
            fprintf(stderr, "load txt2:\n");
            txt2.load("data/x.txt");
            fprintf(stderr, "unload txt1:\n");
            txt1.unload();
            fprintf(stderr, "unload txt2:\n");
            txt2.unload();
            fprintf(stderr, "unload txt3:\n");
            txt3.unload();
            fprintf(stderr, "load txt2:\n");
            txt2.load("data/x.txt");



            //fprintf(stderr, "**%s**\n",txt->txt.c_str());
        }
    rs4::Game * getGame() { return game; }
    World * getWorld() { return &world; }

    void update(int dt)
    {
        while (transition_priority > 0)
        {
            transition_priority = 0;
            currentScreen = transition_fn(currentScreen);
            fprintf(stderr, "New state: \"%s\"\n", typeid(*currentScreen).name());
        }

        //if (!currentScreen->isPaused())
        {
            currentScreen->update(dt);
        }
    }

    void render(float alpha)
    {
        video->startRender();
        currentScreen->render(alpha);
        video->endRender();
    }

    void start()
    {
        currentScreen->start();
    }

    void stop()
    {
        currentScreen->stop();
        // TODO stop all screens
    }

    template <class TEvent>
    void onEvent(const TEvent&) {}

    void onEvent(const EventMenu & e)
    {
        transition(10,
            [this](rs4::IScreen * s)
            {
                screen_menu.setSubscreen(s);
                s->pause();
                screen_menu.start();
                return &screen_menu;
            }
        );
    }

    void onEvent(const EventResume & e)
    {
        transition(10,
            [this](rs4::IScreen *)
            {
                rs4::IScreen * s2 = screen_menu.getSubscreen();
                screen_menu.stop();
                screen_menu.setSubscreen(nullptr);
                s2->unpause();
                return s2;
            }
        );
    }

    void onEvent(const EventGameOver & e) { game->exit(); }

};


#endif // PLANETSTORM18_HPP_INCLUDED
