
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "planet_graphics.hpp"
#include "component.hpp"


GraphicsPlanet<rs4::VideoSDLGL>::GraphicsPlanet(rs4::VideoSDLGL * video, rs4::Game * g, World * w):
        video{video}, world{w}, hajo{g}, ellenseg{g}
{
    using rs4::vertex_shader_source;
    using rs4::fragment_shader_source;
    using rs4::texture_data;
    using rs4::texture_w;
    using rs4::texture_h;

    //glClearColor(0.0f,0.0f,0.8f,1.0f);

    // SHADERS

    shader_program = video->makeShaderProgram(vertex_shader_source,1,
                                              fragment_shader_source,1);


    // VAO, VBO, EBO

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);


    glBindVertexArray(vao[0]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(4*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Uniform

    u_vertex_colour = glGetUniformLocation(shader_program, "ourColour");
    if (u_vertex_colour == -1)
        throw std::runtime_error("glGetUniformLocation failed for vertex colour");
    u_model = glGetUniformLocation(shader_program, "model");
    if (u_model == -1)
        throw std::runtime_error("glGetUniformLocation failed for model");
    u_view = glGetUniformLocation(shader_program, "view");
    if (u_view == -1)
        throw std::runtime_error("glGetUniformLocation failed for view");
    u_projection = glGetUniformLocation(shader_program, "projection");
    if (u_projection == -1)
        throw std::runtime_error("glGetUniformLocation failed for projection");

    // Texture
    GLfloat borderc [] = {1.0f, 0.8f, 0.0f, 1.0f};
    glGenTextures(3, textures);


    // hajo
    hajo.load("data/hajo2.png");
    assert(hajo->pixel_size == 4);

    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderc);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_w, texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hajo->width, hajo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &hajo->pix[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    // ellenseg
    ellenseg.load("data/ellenseg1.png");
    assert(ellenseg->pixel_size == 4);

    glBindTexture(GL_TEXTURE_2D, textures[2]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderc);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_w, texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ellenseg->width, ellenseg->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &ellenseg->pix[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    // felszin
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderc);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_w, texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data[3]);
    glGenerateMipmap(GL_TEXTURE_2D);

}



void GraphicsPlanet<rs4::VideoSDLGL>::render(float alpha)
{

    const std::size_t i1 = world->i1;
    const std::size_t i0 = 1 - i1;

    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);//űr
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    glBindVertexArray(vao[0]);

    glm::mat4 m_model;
    glm::mat4 m_view(1.0f);
    glm::mat4 m_projection = glm::perspective(glm::radians(45.0f), video->aspect, 0.1f, 100.0f);

    Camera &cam = world->registry.get<Camera>();

    m_view = glm::translate(m_view, glm::vec3(-cam.x, 0.0f ,-cam.distance));
    m_view = glm::rotate(m_view, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // SURFACE
    m_model = glm::mat4(1.0f);
    m_model = glm::scale(m_model, glm::vec3(4.0f, 2.0f, 1.0f));

    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(m_model));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(m_view));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(m_projection));
    glUniform4f(u_vertex_colour, 1.0f, 1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

    // PLAYER
    auto pent = world->registry.attachee<Player>();
    Position &pp = world->registry.get<Position>(pent);
    Colour &pc = world->registry.get<Colour>(pent);
    Velocity &pv = world->registry.get<Velocity>(pent);

    float x = interpolate(pp.buf[i0].x,pp.buf[i1].x,alpha);
    float y = interpolate(pp.buf[i0].y,pp.buf[i1].y,alpha);


    m_model = glm::mat4(1.0f);
    m_model = glm::translate(m_model, glm::vec3(x, y, 0.0f));
    m_model = glm::rotate(m_model, glm::atan(-pv.vx, pv.vy), glm::vec3(0.0f, 0.0f, 1.0f));
    m_model = glm::scale(m_model, glm::vec3(0.1f, 0.1f, 0.1f));

    glBindTexture(GL_TEXTURE_2D, textures[1]);

    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(m_model));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(m_view));
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(m_projection));

    glUniform4f(u_vertex_colour, (float)pc.r/255, (float)pc.g/255, (float)pc.b/255, 1.0f);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

    // ENEMIES
	auto view = world->registry.view<Position, Velocity, Colour, Monster>(entt::persistent_t{});

    for(auto entity: view)
    {
        Position &p = view.get<Position>(entity);
        Colour &c = view.get<Colour>(entity);
        x = interpolate(p.buf[i0].x,p.buf[i1].x,alpha);
        y = interpolate(p.buf[i0].y,p.buf[i1].y,alpha);


        m_model = glm::mat4(1.0f);
        m_model = glm::translate(m_model, glm::vec3(x, y, 0.0f));
        m_model = glm::scale(m_model, glm::vec3(0.1f, 0.1f, 0.1f));

        glBindTexture(GL_TEXTURE_2D, textures[2]);

        glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(m_model));
        glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(m_view));
        glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(m_projection));

        glUniform4f(u_vertex_colour, (float)c.r/255, (float)c.g/255, (float)c.b/255, 1.0f);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
        //RS4_GLERRORS;

    }

}



// TEST

void GraphicsPlanet<rs4::VideoTest>::render(float alpha)
{
    //std::size_t idxBufOld = 1 - idxBuf;

	auto view = world->registry.view<Position, Velocity, Colour>(entt::persistent_t{});

    int count=0;
    for(auto entity: view)
    {
        /*Position &p = */view.get<Position>(entity);
        /*Velocity &v = */view.get<Velocity>(entity);
        //fprintf(video->f, "x%f\ty%f\tvx%f\tvy%f\n",p.buf[idxBuf].x,p.buf[idxBuf].y,v.vx,v.vy);
        count++;
    }
    fprintf(video->f, "count=%d\n",count);
}

