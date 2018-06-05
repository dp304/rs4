#include <cstdio>
#include <random>
#include <cmath>

#include "rs4/rs4.hpp"
#include "rs4/rs4_sdlglal.hpp"

#include "planetstorm18.hpp"

static const char * default_config =
R"(
fullscreen = 0
fullscreen_mode = 0
resolution_x = 800
resolution_y = 600
sound = 1
music = 1
volume_master = 100
volume_sound = 100
volume_music = 50
)";

/*static const rs4::Meta meta =
{
    title : "Planet Storm 18",
    version : "0.0"
};*/

static rs4::Meta meta;

//??
#ifdef _WIN32
#undef main
#endif

int main(int argc, char** argv) {
	meta.title = "Planet Storm 18";
	meta.version = "0.0";

    try
    {
        rs4::Engine<rs4::PlatformSDLGLAL, PlanetStorm18> engine(meta, default_config);
        //rs4::Engine<rs4::PlatformTest, PlanetStorm18> engine(meta, default_config);
        engine.loop();
    }
    catch (std::exception & error)
    {
        fprintf(stderr,"FATAL: %s\n", error.what());
    }
    return 0;
}
