#version 100

precision highp float;

uniform sampler2D baseColorTexture;

varying vec2 vs_texCoord;
varying vec4 vs_color;

void main(void)
{
    float alpha = texture2D(baseColorTexture, vs_texCoord).r;
    vec4 color = vs_color;
    color.a *= alpha;
    gl_FragColor = color;
}
