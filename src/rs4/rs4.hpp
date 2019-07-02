#ifndef RS4_HPP_INCLUDED
#define RS4_HPP_INCLUDED

#include <memory>
#include <tuple>
#include <map>
#include <unordered_map>
#include <list>
#include <regex>
#include <string>
#include <sstream>
#include <functional>
#include <cassert>
#include <stdexcept>
#include <cstdio>
#include <cstring>


namespace rs4
{

class Game;

// STREAM INTERFACE

class IStream
{
    bool _readable = false;
    bool _writable = false;
    long long _size = -1;
    bool _eof = false;

    virtual void onOpenr() = 0;
    virtual void onOpenw() = 0;
    virtual void onClose() = 0;
    virtual std::size_t onRead(void * buf, std::size_t siz, std::size_t num) = 0;
    virtual std::size_t onWrite(const void * buf, std::size_t siz, std::size_t num) = 0;
    virtual long long onTell() { return -1; }
    virtual bool onSeek(long long offset, int whence) { return false; }
protected:
    void setSize(long long siz) { _size = siz; }
public:
    void openr()
    {
        assert(!_readable && !_writable);
        onOpenr();
        _readable = true;
    }
    void openw()
    {
        assert(!_readable && !_writable);
        onOpenw();
        _writable = true;
    }
    void close()
    {
        assert(_readable || _writable);
        onClose();
        _size = -1;
        _readable = false;
        _writable = false;
        _eof = false;
    }
    bool isReadable() const { return _readable; }
    bool isWritable() const { return _writable; }
    bool isOpen() const { return _readable || _writable; }
    bool eof() const { assert(_readable); return _eof; }
    long long size() const { assert(_readable); return _size; }
    long long tell() { assert(_readable); return onTell(); }
    bool seek(long long offset, int whence) { assert(_readable); return onSeek(offset, whence); }
    void rewind() { assert(_readable); if (!seek(0,SEEK_SET)) {close(); openr();} }
    std::size_t read(void * buf, std::size_t siz, std::size_t num)
    {
        assert(_readable);
        const std::size_t n = onRead(buf, siz, num);
        if (n==0) _eof = true;
        return n;
    }
    std::size_t write(const void * buf, std::size_t siz, std::size_t num)
    {
        assert(_writable);
        return onWrite(buf, siz, num);
    }


    bool getline(std::string * l, std::size_t maxcn = 256)
    {
        l->clear();
        std::size_t i, cn = 0;
        char c;
        while ( (i = read(&c, sizeof(char), 1)) == 1 && c != '\n' )
        {
            cn++;
            if (cn > maxcn)
                throw std::runtime_error("Line too long");
            *l += c;
        }
        return i==1;
    }
    void putline(const std::string & l)
    {
        static const char eol = '\n';
        write(l.c_str(), sizeof(char), l.length());
        write(&eol, sizeof(char), 1);
    }
    virtual ~IStream() { }
};

class StreamNull : public IStream
{
    void onOpenr() final { setSize(0); }
    void onOpenw() final { }
    void onClose() final { }
    long long onTell() final { return 0; }
    bool onSeek(long long, int) final { return 0; }
    std::size_t onRead(void * buf, std::size_t siz, std::size_t num) final
    {
        return 0;
    }
    std::size_t onWrite(const void * buf, std::size_t siz, std::size_t num) final
    {
        return num;
    }
};

class StreamMem : public IStream
{
public:
    StreamMem(const void * ptr, std::size_t siz):_ptr{ptr},_i{0},_bufSiz{siz} {}
private:
    const void * const _ptr;
    std::size_t _i, _bufSiz;


    void onOpenr() final { setSize(_bufSiz); _i = 0; }
    void onOpenw() final { assert(false&&"StreamMem can only read memory"); }
    void onClose() final { }
    long long onTell() final { return _i; }
    bool onSeek(long long offset, int whence) final
    {
        std::size_t from = (whence==SEEK_CUR ? _i : (whence==SEEK_END ? _bufSiz : 0) );
        _i = from + (std::size_t)offset;
        if (_i < 0) _i = 0;
        else if (_i > _bufSiz) _i = _bufSiz;
        return true;
    }
    std::size_t onRead(void * buf, std::size_t siz, std::size_t num) final
    {
        if (_i + num * siz >= _bufSiz)   // TODO '>' ?
            num = (_bufSiz - _i)/siz;

        std::size_t nbyte = num * siz;

        std::copy( (char*) _ptr + _i, (char*) _ptr + _i + nbyte, (char*) buf);
        _i += nbyte;

        return num;
    }
    std::size_t onWrite(const void * buf, std::size_t siz, std::size_t num) final
    {
        return 0;
    }
};

// DATA, DATA STORE

struct ILoadedData
{
    virtual ~ILoadedData() {}
};

class DataStore
{
public:
    typedef std::function<std::unique_ptr<IStream>(const char *) > stream_factory_t;
private:
    std::unordered_map<std::string, std::weak_ptr<ILoadedData> > _datamap;
    std::list<std::pair<std::regex, stream_factory_t> > _sources;
public:
    std::unique_ptr<IStream> makeStream(const char * path)
    {
        auto it = std::find_if(_sources.begin(),
                              _sources.end(),
                              [path](const std::pair<std::regex, stream_factory_t> &s) -> bool
                              {
                                    return std::regex_match(path, s.first);
                              }
                              );
        if (it == _sources.end())
            throw std::runtime_error(std::string("Unknown source for data \"")+path+"\"");

        return (it->second)(path);
    }

    template <class TLoadedData>
    std::shared_ptr<ILoadedData> load(const char * path)
    {
        // TODO purge buckets
        std::weak_ptr<ILoadedData> & wp = _datamap[path];
        std::shared_ptr<ILoadedData> sp = wp.lock();
        if (!sp)
        {
            auto stream = makeStream(path);
            stream->openr();
            sp = TLoadedData::load(stream.get(), path);
            stream->close();
            wp = sp;
        }
        else if (dynamic_cast<TLoadedData*>(sp.get()) == nullptr)
        {
            throw std::runtime_error(
                std::string("Invalid format ")+typeid(TLoadedData).name()+
                " for data \""+path+"\" already loaded as "+
                typeid(*sp).name()
                );
        }

        return sp;
    }

    void addSource(const char * re, stream_factory_t sf)
    {
        _sources.emplace_front(re,sf);
    }

    void addSourceFallback(const char * re, stream_factory_t sf)
    {
        _sources.emplace_back(re,sf);
    }


};

template <class TLoadedData>
class Data
{
    DataStore * _store;
    std::shared_ptr<ILoadedData> _ptr;
public:
    Data(DataStore * ds):_store{ds} {}
    Data(Game * g);
    TLoadedData * operator->()
    {
        return static_cast<TLoadedData*>(_ptr.get());  // Downcast!
    }
    bool isLoaded() { return _ptr != nullptr; }
    void load(const char * path)
    {
        if (_ptr) return;
        _ptr = _store->load<TLoadedData>(path);
    }
    void unload()
    {
        _ptr.reset();
    }

};

// LOADED DATA

struct LDText: public ILoadedData
{
    std::string txt;
    static std::shared_ptr<LDText> load(IStream * stream, const char * path)
    {
        fprintf(stderr, "Loading text \"%s\"...\n", path);
        long long siz = stream->size();
        if (siz<0)
            throw std::runtime_error(std::string("Text loading implemented only for streams with size"));
        else if (siz>0xFFFFFFFF)
            throw std::runtime_error(std::string("Text too long"));

        auto ldp = std::make_shared<LDText>();

        ldp->txt.resize((std::size_t)siz);

        stream->read((void*)(ldp->txt.data()), 1, (std::size_t)siz);

        return ldp;
    }
};

struct LDImage: public ILoadedData
{
    int width, height;
    int pixel_size;
    std::vector<char> pix;
    static std::shared_ptr<LDImage> load(IStream * stream, const char * path);
};

struct LDSound: public ILoadedData
{
    int channels;
    int rate;
    int sample_size;
    std::vector<char> samples;
    static std::shared_ptr<LDSound> load(IStream * stream, const char * path);
};


// CONFIG

class ConfigValue
{
    friend class Config;

    enum type_t {INVALID, NONE, INTEGER, FLOAT, STRING} type = NONE;
    union
    {
        int i;
        float f;
    } value;
    std::string value_s;

    std::list<std::function<void(const ConfigValue &)> > callbacks;

	ConfigValue(int val):type{INTEGER} { value.i = val; }
	ConfigValue(float val):type{FLOAT} { value.f = val; }
    ConfigValue(const std::string & val):type{STRING},value_s{val} {}

    void notifyAll()
    {
        for (auto it = callbacks.begin(); it!=callbacks.end(); it++)
            (*it)(*this);
    }

    void checkType(type_t t) const
    {
        if (type != t) throw std::runtime_error("Type mismatch");
    }

    void checkType(type_t t)
    {
        if (type == NONE) type = t;
        if (type != t) throw std::runtime_error("Type mismatch");
    }

    void set(int val) { checkType(INTEGER); value.i=val; notifyAll(); }
    void set(float val) { checkType(FLOAT); value.f=val; notifyAll(); }
    void set(const std::string & val) { checkType(STRING); value_s=val; notifyAll(); }

public:
    ConfigValue()=default;

    int getI() const { checkType(INTEGER); return value.i; }
    float getF() const { checkType(FLOAT); return value.f; }
    const std::string & getS() const { checkType(STRING); return value_s; }

    operator int () const { return getI(); }
    operator float () const { return getF(); }
    operator const std::string & () const { return getS(); }
};


class Config
{
    typedef std::map<std::string, ConfigValue> store_t;

    store_t store;
    bool dirty=false;

    std::unique_ptr<IStream> stream;

    void parseline(std::string * l)
    {
        if (l->empty()) return;
        std::size_t eqi = l->find('=');
        if (eqi == std::string::npos)
            throw std::runtime_error("Missing '='");
        if (eqi == 0)
            throw std::runtime_error("Missing key");
        std::size_t kendi = l->find_last_not_of(' ', eqi - 1);
        if (kendi == std::string::npos)
            throw std::runtime_error("Missing key");
        std::size_t kstarti = l->find_first_not_of(' ');
        std::size_t vstarti = l->find_first_not_of(' ', eqi + 1);
        if (vstarti == std::string::npos)
            throw std::runtime_error("Missing value");
        std::size_t vendi = l->find_last_not_of(' ');

        std::string key = l->substr(kstarti, kendi-kstarti+1);
        std::string value = l->substr(vstarti, vendi-vstarti+1);

        if (value.front()=='"' && value.back()=='"')
        {
            // STRING VALUE
            value = value.substr(1, value.size() - 2);
            store[key].set(value);
        }
        else if (    (value.front()=='-' && value.find_first_not_of("0123456789",1) == std::string::npos)
                  || (value.find_first_not_of("0123456789",0) == std::string::npos) )
        {
            // INTEGER VALUE
            int vi = std::stoi(value);
            store[key].set(vi);
        }
        else if (    (value.front()=='-' && value.find_first_not_of("0123456789.",1) == std::string::npos)
                  || (value.find_first_not_of("0123456789.",0) == std::string::npos) )
        {
            // FLOAT VALUE
            float vf = std::stof(value);
            store[key].set(vf);
        }
        else
            throw std::runtime_error("Missing '\"'");

    }

    const ConfigValue & safe_find(const std::string & key) const
    {
        auto it = store.find(key);
        if ( it == store.end() )
        {
            throw std::runtime_error("Unknown key \"" + key + "\"");
        }
        return it->second;
    }

    ConfigValue & safe_find(const std::string & key)
    {
        return const_cast<ConfigValue &>(const_cast<const Config *>(this)->safe_find(key));
    }


public:
    Config():stream{new StreamNull} {}
    Config(const char * cfg_s):stream{new StreamMem(cfg_s, strlen(cfg_s))} {load();}
    void setStream(IStream * s) { stream.reset(s); }

    bool isDirty() const { return dirty; }
    void setDirty() { dirty = true; }

    const ConfigValue & get(const std::string & key) const
    {
        return safe_find(key);
    }

    template<class TValue>
    void set(const std::string & key, const TValue & value)
    {
        safe_find(key).set(value);
        dirty=true;
    }

    const ConfigValue & subscribe(const std::string & key,
                                  std::function<void(const ConfigValue &)> cb,
                                  bool notifyNow = true)
    {
        ConfigValue & val = safe_find(key);
        val.callbacks.push_back(cb);
        if (notifyNow) cb(val);
        return val;
    }

    template<class TValue>
    const ConfigValue & subscribe(const std::string & key, TValue * v)
    {
        return subscribe(key, [v](const ConfigValue & val) { *v = val; });
    }

    void load()
    {
        stream->openr();
        std::string line;
        int ln = 1;
        bool more = true;
        try
        {
            do
            {
                more = stream->getline(&line);
                parseline(&line);
                ln++;
            }
            while (more);
        }
        catch (std::runtime_error & err)
        {
            stream->close();
            std::stringstream ss;
            ss<<"Error parsing configuration at line " << ln << ": " << err.what();
            throw std::runtime_error(ss.str());
        }
        stream->close();
        dirty = false;
    }
    void save()
    {
        std::string line;
        stream->openw();
        for (const auto & ent : store)
        {
            line = ent.first + " = ";
            switch (ent.second.type)
            {
            case ConfigValue::INTEGER:
                line += std::to_string(ent.second.getI());
                break;
            case ConfigValue::FLOAT:
                line += std::to_string(ent.second.getF());
                break;
            case ConfigValue::STRING:
                line = line + "\"" + ent.second.getS() + "\"";
                break;
            default:
                assert(false && "Invalid type");
            }
            stream->putline(line);
        }
        stream->close();

        dirty = false;

    }
};



// DEFAULT PLATFORM

struct PlatformTest;
class ClockTest;
class DiskTest;
class AudioTest;
class VideoTest;
class InputTest;

template<class TPlatform> class MachineTest;

// GAME, ENGINE ETC.

struct Meta
{
    std::string title = "rs4 game";
    std::string version = "0.0";
    std::string author = "Electric Elephant";
    std::string date = __DATE__;
    std::string time = __TIME__;
};


class Game
{
    template<class TPlatform,
             template<class> class TMachine,
             int Tdt, int Tminframe, int Tmaxupdates> friend class Engine;
    bool exiting;

public:
    const Meta meta;
    Config config;
    DataStore ds;

    Game(const Game&) = delete;
    Game(const Meta & m, const char * cfg_s):exiting{false},meta{m},config{cfg_s} { }

    void exit()
    {
        exiting = true;
    }

    ~Game() { }
};

// convenience constructor for Data
template <class TLoadedData>
Data<TLoadedData>::Data(Game * g):_store{&g->ds} {}
//

template<class TPlatform=PlatformTest,
         template<class> class TMachine=MachineTest,
         int Tdt=10, int Tminframe=10, int Tmaxupdates=20>
class Engine
{
    Game game;
    TPlatform platform;
    typename TPlatform::Clock clock;
    typename TPlatform::Disk disk;
    typename TPlatform::Audio audio;
    typename TPlatform::Video video;
    typename TPlatform::Input input;
    TMachine<TPlatform> machine;

    bool running;

public:

    Engine(const Engine&) = delete;
    Engine(const Meta & m, const char * cfg_s = ""):
        game(m, cfg_s),
        platform(&clock,&disk,&audio,&video,&input),
        clock(&platform),
        disk(&platform, &game),
        audio(&platform, &game),
        video(&platform, &game),
        input(&platform),
        machine(&game, &platform),
        running{false}
    { }

    void loop()
    {
        unsigned long t;
        int updates_left;
        float alpha;

        assert(!running&&"Re-entry into loop");
        running = true;

        machine.start();

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
                platform.handleEvents(&game);

                machine.update(Tdt);

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

            alpha = (float)t/Tdt;
            machine.render(alpha);
        }
        while (true);

        machine.stop();
        running = false;

    }
    ~Engine() {}
};

// TEST

struct PlatformTest
{
    typedef ClockTest Clock;
    typedef DiskTest Disk;
    typedef AudioTest Audio;
    typedef VideoTest Video;
    typedef InputTest Input;

    Clock * clock;
    Disk * disk;
    Audio * audio;
    Video * video;
    Input * input;

    PlatformTest(Clock*c,Disk*d,Audio*a,Video*v,Input*i):clock{c},disk{d},audio{a},video{v},input{i} {}
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

class DiskTest
{
public:
    DiskTest(PlatformTest * platform, Game * game) {}
};

class AudioTest
{
public:
    FILE * f;
    AudioTest(const AudioTest &) = delete;
    AudioTest(PlatformTest * platform, Game * game)
    {
        f = stderr;
    }
    ~AudioTest() {}
};

class VideoTest
{
public:
    FILE * f;
    VideoTest(const VideoTest &) = delete;
    VideoTest(PlatformTest * platform, Game * game)
    {
        f = stderr;
    }
    void startRender() {}
    void endRender() {}
    ~VideoTest() {}
};



class InputTest
{
public:
    InputTest(PlatformTest * platform) {}
};

/////////////////

template<class TPlatform>
class MachineTest
{
    Game * game;
    int t;
public:
    MachineTest(Game * g, TPlatform * platform):game(g),t{0} {};
    void update(int dt)
    {
        if ((t+=dt)>10000) game->exit();
    }
    void render(float alpha);
};

template<class TPlatform>
inline void MachineTest<TPlatform>::render(float alpha)
{
    fprintf(stderr, "\t%f\n",alpha);
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

    void start()
    {
        assert(!started_);
        onStart();
        started_ = true;
    }
    void pause() {
        assert(started_);
        assert(!paused_);
        onPause();
        paused_ = true;
    }
    void unpause() {
        assert(started_);
        assert(paused_);
        onUnpause();
        paused_ = false;
    }
    void stop() {
        assert(started_);
        if (paused_) unpause();
        onStop();
        started_ = false;
    }

    virtual void update(int dt) = 0;
    virtual void render(float alpha) = 0;

    virtual const char * debugName() const { return "UNNAMED_STATE"; }

    virtual ~IScreen() {}

private:
    virtual void onStart() = 0;
    virtual void onPause() = 0;
    virtual void onUnpause() = 0;
    virtual void onStop() = 0;
};


/// Event broadcaster
template<class ...TObservers>
class Caster
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
    Caster(TObservers* ...obs):observers{obs...} {}
    template<class TEvent>
    void signal(const TEvent& event)
    {
        ForEach<TEvent>::signal(observers, event);
    }
};

} // namespace rs4

#endif // RS4_HPP_INCLUDED
