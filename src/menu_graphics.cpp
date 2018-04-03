
#include "menu_graphics.hpp"

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



GraphicsMenu<rs4::VideoSDLGL>::GraphicsMenu(rs4::VideoSDLGL * video):
        video{video}
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

        glUseProgram(shader_program);
        glBindVertexArray(vao[0]);

        glUniform4f(u_fade_colour, 0.0f, 0.0f, 0.0f, 0.5f*phase/1000.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        glDisable(GL_BLEND);

        //RS4_GLERRORS
    }
    else
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

}



// TEST

void GraphicsMenu<rs4::VideoTest>::render(float alpha)
{
    //std::size_t idxBufOld = 1 - idxBuf;

}
