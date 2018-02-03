#ifndef RS4_HPP_INCLUDED
#define RS4_HPP_INCLUDED

#include <tuple>
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
    typename TPlatform::Audio audio;
    typename TPlatform::Video video;
    typename TPlatform::Input input;
    TState<TPlatform> state;

    bool running;

    std::size_t idxBuf;

public:

    Engine(const Engine&) = delete;
    Engine():
        platform(&clock,&audio,&video,&input),
        clock(&platform),
        audio(&platform),
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

/// Game screen (or state) interface.
class IScreen
{
    bool started_;
    bool paused_;

public:
    IScreen():started_{false},paused_{false} {}
    bool isStarted() { return started_; }
    bool isPaused() { return paused_; }

    void start(std::size_t i1) { started_ = true; onStart(i1); }
    void pause(std::size_t i1) { paused_ = true; onPause(i1); }
    void unpause(std::size_t i1) { onUnpause(i1); paused_ = false; }
    void stop(std::size_t i1) { onStop(i1); started_ = false; }

    virtual void update(int dt, std::size_t i1) = 0;
    virtual void render(float alpha, std::size_t i1) = 0;

private:
    virtual void onStart(std::size_t i1) = 0;
    virtual void onPause(std::size_t i1) {}
    virtual void onUnpause(std::size_t i1) {}
    virtual void onStop(std::size_t i1) {}

};

/// Event dispatcher
template<class ...TObservers>
class EventDispatcher
{
    std::tuple<TObservers*...> observers;

    template<class TEvent, std::size_t Idx = sizeof...(TObservers)>
    struct ForEach
    {
        static inline void signal(decltype(observers) & obs, const TEvent & event)
        {
            std::get<sizeof...(TObservers)-Idx>(obs) -> onEvent(event);
            ForEach<TEvent, Idx-1>::signal(obs, event);
        }
    };

    template<class TEvent>
    struct ForEach<TEvent, 0>
    {
        static inline void signal(decltype(observers) & obs, const TEvent & event)
        {
        }
    };

public:
    EventDispatcher(TObservers* ...obs):observers{obs...} {}
    template<class TEvent>
    void signal(const TEvent& event)
    {
        ForEach<TEvent>::signal(observers, event);
    }
};

} // namespace rs4

#endif // RS4_HPP_INCLUDED
