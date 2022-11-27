#version 100
#extension GL_OES_standard_derivatives : enable

precision highp float;

varying vec2 vs_texCoord;
varying vec4 vs_color;

void main(void)
{
    const float Radius = 0.5;
    float feather = 2.0 * max(fwidth(vs_texCoord.x), fwidth(vs_texCoord.y));
    float d = distance(vs_texCoord, vec2(0.5, 0.5));
    float alpha = smoothstep(Radius, Radius - feather, d);
    gl_FragColor = vec4(vs_color.xyz, alpha * vs_color.w);
}
