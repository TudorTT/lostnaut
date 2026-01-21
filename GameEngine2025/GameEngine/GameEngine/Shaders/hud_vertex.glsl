#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 MVP;

void main()
{
    TexCoord = texCoord;
    gl_Position = MVP * vec4(pos, 1.0f);
}