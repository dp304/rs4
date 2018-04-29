
#include "menu_graphics.hpp"


#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/demo/sdl_opengl3/nuklear_sdl_gl3.h"

const char * GraphicsMenu<rs4::VideoSDLGL>::vshader_src =
R"(
#version 330 core
layout (location = 0) in vec3 aPos;
//out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

const char * GraphicsMenu<rs4::VideoSDLGL>::fshader_src =
R"(
#version 330 core
out vec4 FragColour;
uniform vec4 fadeColour;
//in vec2 TexCoord;
//uniform sampler2D ourTexture;

void main()
{
    FragColour = vec4(fadeColour);
}
)";



GraphicsMenu<rs4::VideoSDLGL>::GraphicsMenu(rs4::VideoSDLGL * video, UIMenu * ui):
        video{video}, ui{ui}
{
    shader_program = video->makeShaderProgram(&vshader_src,1,&fshader_src,1);

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);

    glBindVertexArray(vao[0]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    u_fade_colour = glGetUniformLocation(shader_program, "fadeColour");
    if (u_fade_colour == -1)
        throw std::runtime_error("glGetUniformLocation failed for fade colour");

    ui->context = nk_sdl_init(video->window);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_sdl_font_stash_end();
    /*nk_style_load_all_cursors(ui->context, atlas->cursors);*/
    /*nk_style_set_font(ui->context, &tiny->handle);*/}

    ui->bg.r = 0.0f, ui->bg.g = 0.0f, ui->bg.b = 0.0f, ui->bg.a = 1.0f;



}


void GraphicsMenu<rs4::VideoSDLGL>::update(std::size_t dt)
{
    if (phase<1000) phase+=dt;



}


void GraphicsMenu<rs4::VideoSDLGL>::render(float alpha)
{


    if (subscreen != nullptr)
    {
        subscreen->render(1.0f);

        ui->beginRendering();
        ui->update();
        ui->endRendering();


        glUseProgram(shader_program);
        glBindVertexArray(vao[0]);

        glUniform4f(u_fade_colour, ui->bg.r, ui->bg.g, ui->bg.b, 0.5f*phase/1000.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glDisable(GL_BLEND);

        //RS4_GLERRORS




        nk_sdl_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);
        video->updateAspect();

    }
    else
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

}

GraphicsMenu<rs4::VideoSDLGL>::~GraphicsMenu()
{
    nk_sdl_shutdown();
}

// TEST

void GraphicsMenu<rs4::VideoTest>::render(float alpha)
{
    //std::size_t idxBufOld = 1 - idxBuf;

}
