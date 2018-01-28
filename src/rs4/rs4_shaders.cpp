namespace rs4
{

const char * vertex_shader_source[] =
{
R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aGB;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out float ourGB;
out vec2 TexCoord;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourGB = aGB;
    TexCoord = aTexCoord;
}
)"
};

const char * fragment_shader_source[] =
{
R"(
#version 330 core
out vec4 FragColour;
uniform vec4 ourColour;
in float ourGB;
in vec2 TexCoord;
uniform sampler2D ourTexture;

void main()
{
    //FragColour = vec4(ourColour.r, ourColour.g*ourGB, ourColour.b*ourGB, 1.0);
    //FragColour = texture(ourTexture, TexCoord) * ourColour;
    FragColour = mix(ourColour, texture(ourTexture, TexCoord), texture(ourTexture, TexCoord).a);
    //vec4 tex = texture(ourTexture, TexCoord);
    //if (tex.a < 0.1f)
    //    discard;
    //if (tex.a < 0.5f)
    //    tex *= ourColour;
    //FragColour = tex;

}


)"
};

}
