#version 100

attribute vec2 position;
attribute vec2 texCoord;
attribute vec4 color;

uniform mat4 mvp;

varying vec2 vs_texCoord;
varying vec4 vs_color;

void main(void)
{
    vs_texCoord = texCoord;
    vs_color = color;
    gl_Position = mvp * vec4(position, 0.0, 1.0);
}
