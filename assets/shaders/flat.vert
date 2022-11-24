#version 100

attribute vec2 position;
attribute vec4 color;

uniform mat4 mvp;

varying vec4 vs_color;

void main(void)
{
    vs_color = color;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}
