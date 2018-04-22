#include <cstdio>
#include <random>
#include <cmath>

#include "rs4/rs4.hpp"
#include "rs4/rs4_sdlgl.hpp"

#include "planetstorm18.hpp"

static const char * default_config =
R"(
resolution_x = 800
resolution_y = 600
sound = 1
)";

static const rs4::Meta meta =
{
    title : "Planet Storm 18",
    version : "0.0"
};

int main() {

    try
    {
        rs4::Engine<rs4::PlatformSDLGL, PlanetStorm18> engine(meta, default_config);
        //rs4::Engine<rs4::PlatformTest, PlanetStorm18> engine(default_config);
        engine.loop();
    }
    catch (std::runtime_error & error)
    {
        fprintf(stderr,"FATAL: %s\n", error.what());
    }
    return 0;
}
