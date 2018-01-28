#ifndef RS4_HPP_INCLUDED
#define RS4_HPP_INCLUDED

#include <cassert>
#include <stdexcept>
#include <cstdio>


namespace rs4
{


class PlatformTest;
class ClockTest;
class VideoTest;
class InputTest;

template<class TPlatform> class StateTest;

class Game
{
    template<class TPlatform,
             template<class> class TState,
             int Tdt, int Tminframe, int Tmaxupdates> friend class Engine;
    bool exiting;

public:
    Game(const Game&) = delete;
    Game():exiting{false} { }

    void exit()
    {
        exiting = true;
    }

    ~Game() { }
};


template<class TPlatform=PlatformTest,
         template<class> class TState=StateTest,
         int Tdt=10, int Tminframe=10, int Tmaxupdates=20>
class Engine
{
    Game game;
    TPlatform platform;
    typename TPlatform::Clock clock;
    typename TPlatform::Video video;
    typename TPlatform::Input input;
    TState<TPlatform> state;

    bool running;

    std::size_t idxBuf;

public:

    Engine(const Engine&) = delete;
    Engine():
        platform(&clock,&video,&input),
        clock(&platform),
        video(&platform),
        input(&platform),
        state(&game, &platform),
        running{false},
        idxBuf{0}
    { }

    void loop()
    {
        unsigned long t;
        int updates_left;
        double alpha;

        assert(!running&&"Re-entry into loop");
        running = true;

        clock.reset();
        t = 0;
        do
        {
            t += clock.elapsed();
            if (t<Tminframe)
            {
                clock.sleep(Tminframe-t);
                t += clock.elapsed();
            }

            updates_left = Tmaxupdates;
            while (t >= Tdt && updates_left > 0 && !game.exiting)
            {
                idxBuf = 1-idxBuf;
                platform.handleEvents(&game);

                state.update(Tdt, idxBuf);

                updates_left--;
                t -= Tdt;
                if (t < Tdt)
                    t += clock.elapsed();
            }

            if (game.exiting)
                break;

            if (updates_left == 0 && t > Tdt) {
                t = Tdt;
            }

            alpha = (double)t/Tdt;
            state.render(alpha, idxBuf);
        }
        while (true);

        running = false;

    }
    ~Engine() {}
};

struct PlatformTest
{
    typedef ClockTest Clock;
    typedef VideoTest Video;
    typedef InputTest Input;

    Clock * clock;
    Video * video;
    Input * input;

    PlatformTest(Clock*c,Video*v,Input*i):clock{c},video{v},input{i} {}
    void handleEvents(Game * game) {}
};

class ClockTest
{
    unsigned long clock, newClock;

public:
    ClockTest(PlatformTest * platform):clock(0), newClock(0) {}
    void reset()
    {
        clock=newClock;
    }
    unsigned long sleep(unsigned long t)
    {
        newClock += t;
        return t;
    }
    unsigned long elapsed()
    {
        unsigned long t=newClock-clock;
        clock=newClock;
        return t;
    }

};

class VideoTest
{
public:
    FILE * f;
    VideoTest(const VideoTest &) = delete;
    VideoTest(PlatformTest * platform)
    {
        f = stderr;
    }
    ~VideoTest() {}
};



class InputTest
{
public:
    InputTest(PlatformTest * platform) {}
};

/////////////////

template<class TPlatform>
class StateTest
{
    Game * game;
    int t;
public:
    StateTest(Game * g, TPlatform * platform):game(g),t{0} {};
    void update(int dt, std::size_t idxBuf)
    {
        if ((t+=dt)>10000) game->exit();
    }
    void render(float alpha, std::size_t idxBuf);
};

template<class TPlatform>
inline void StateTest<TPlatform>::render(float alpha, std::size_t idxBuf)
{
    fprintf(stderr, "%lu\t%f\n",idxBuf,alpha);
}

}

#endif // RS4_HPP_INCLUDED
