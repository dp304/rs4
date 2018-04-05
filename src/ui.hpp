#ifndef UI_HPP_INCLUDED
#define UI_HPP_INCLUDED

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
//#define NK_IMPLEMENTATION
//#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/demo/sdl_opengl3/nuklear_sdl_gl3.h"

struct UIMenu
{
    nk_context * context = nullptr;
    nk_colorf bg;

    enum { START, EVENTS, RENDERING } state = START;

    void beginInput()
    {
        if (state != EVENTS)
        {
            nk_input_begin(context);
            state = EVENTS;
        }
    }
    void endInput()
    {

    }

    void beginRendering()
    {
        if (state == RENDERING)
        {
            nk_input_begin(context);
            nk_input_end(context);
        }
        else
        {
            nk_input_end(context);
            state = RENDERING;
        }
    }

    void endRendering()
    {

    }
};

#endif // UI_HPP_INCLUDED
