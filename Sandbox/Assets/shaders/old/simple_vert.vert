#version 410 core

layout (location = 0) in vec3 position;
out vec3 vpos;
uniform mat4 model = mat4(1.0);
void main()
{
    vpos = position;
    gl_Position = model * vec4(position, 1.0);
}