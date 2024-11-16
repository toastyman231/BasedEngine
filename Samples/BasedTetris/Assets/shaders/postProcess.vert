#version 410 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoords;

out vec2 uvs;

uniform mat4 model = mat4(1.0);

void main()
{
    uvs = texcoords;
    gl_Position = model * vec4(position, 0.0, 1.0);
}