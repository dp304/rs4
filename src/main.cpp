#include <cstdio>
#include <random>
#include <cmath>

#include "rs4/rs4.hpp"
#include "rs4/rs4_sdlgl.hpp"

#include <entt/entt.hpp>

#include "system.hpp"
#include "component.hpp"
#include "renderer.hpp"

int main() {

    try
    {
        rs4::Engine<rs4::PlatformSDLGL/*rs4::PlatformTest*/, ScreenMain> engine;
        engine.loop();
    }
    catch (std::runtime_error & error)
    {
        fprintf(stderr,"FATAL: %s\n", error.what());
    }
    return 0;
}
