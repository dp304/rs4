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
    rs4::VideoSDLGL * video;
    rs4::Config * config;

    nk_context * context = nullptr;

    nk_colorf bg;
    int sound;
    int music;
    int volume_master;
    int volume_sound;
    int volume_music;
    int fullscreen;
    int fullscreen_mode;
    std::vector<const char *> mode_labels = {"desktop"};

    enum { START, EVENTS, RENDERING } state = START;

    UIMenu(rs4::VideoSDLGL * v, rs4::Config * c):video{v},config{c}
    {

        config->subscribe("fullscreen", &fullscreen);
        config->subscribe("fullscreen_mode", &fullscreen_mode);
        config->subscribe("sound", &sound);
        config->subscribe("music", &music);
        config->subscribe("volume_master", &volume_master);
        config->subscribe("volume_sound", &volume_sound);
        config->subscribe("volume_music", &volume_music);

        for (const auto & it : video->modes)
        {
            mode_labels.push_back(it.name.c_str());
        }


    }

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

    void update()
    {
        if (nk_begin(context, "Menü", nk_rect(200, 50, 400, 300),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_dynamic(context, 30, 2);
            nk_label(context, "Master volume", NK_TEXT_LEFT);
            nk_size volume_master_nks = volume_master;
            if (nk_progress(context, &volume_master_nks, 100, NK_MODIFIABLE))
            {
                config->set("volume_master", (int)volume_master_nks);
            }
            if (nk_checkbox_label(context, "Sound", &sound))
            {
                config->set("sound", sound);
            }
            nk_size volume_sound_nks = volume_sound;
            if (nk_progress(context, &volume_sound_nks, 100, NK_MODIFIABLE))
            {
                config->set("volume_sound", (int)volume_sound_nks);
            }
            if (nk_checkbox_label(context, "Music", &music))
            {
                config->set("music", music);
            }
            nk_size volume_music_nks = volume_music;
            if (nk_progress(context, &volume_music_nks, 100, NK_MODIFIABLE))
            {
                config->set("volume_music", (int)volume_music_nks);
            }

            if (nk_checkbox_label(context, "Full screen", &fullscreen))
            {
                config->set("fullscreen", fullscreen);
            }
            int fullscreen_mode_old = fullscreen_mode;
            fullscreen_mode = nk_combo(context, &mode_labels[0], mode_labels.size(), fullscreen_mode, 25, nk_vec2(100,200));
            if (fullscreen_mode != fullscreen_mode_old)
            {
                config->set("fullscreen_mode", fullscreen_mode);
            }


            //nk_layout_row_static(context, 30, 80, 1);
            nk_layout_row_dynamic(context, 30, 1);
            if (nk_button_label(context, "button"))
                fprintf(stderr,"button pressed!\n");
            nk_layout_row_dynamic(context, 30, 2);
            if (nk_option_label(context, "easy", op == EASY)) op = EASY;
            if (nk_option_label(context, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(context, 22, 1);
            nk_property_int(context, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(context, 20, 1);
            nk_label(context, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(context, 25, 1);
            if (nk_combo_begin_color(context, nk_rgb_cf(bg), nk_vec2(nk_widget_width(context),400))) {
                nk_layout_row_dynamic(context, 120, 1);
                bg = nk_color_picker(context, bg, NK_RGBA);
                nk_layout_row_dynamic(context, 25, 1);
                bg.r = nk_propertyf(context, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(context, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(context, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(context, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(context);
            }
        }
        nk_end(context);
    }

    void endRendering()
    {

    }
};

#endif // UI_HPP_INCLUDED
