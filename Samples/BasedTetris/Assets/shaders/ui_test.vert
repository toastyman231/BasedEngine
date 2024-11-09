#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoords;
out vec2 uvs;

uniform mat4 projection = mat4(1.0);
//uniform mat4 view = mat4(1.0);
uniform mat4 model = mat4(1.0);
void main()
{
    uvs = texcoords;
    gl_Position = projection * model * vec4(position.xy, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy, 0.0, 1.0);
}