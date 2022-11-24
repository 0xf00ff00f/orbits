#version 100

precision highp float;

varying vec4 vs_color;

void main(void)
{
    gl_FragColor = vs_color;
}
