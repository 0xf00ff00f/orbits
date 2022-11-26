#version 100

precision highp float;

uniform sampler2D baseColorTexture;

varying vec2 vs_texCoord;
varying vec4 vs_color;

void main(void)
{
    vec4 baseColor = texture2D(baseColorTexture, vs_texCoord);
    gl_FragColor = baseColor * vs_color;
}
