#version 100
#extension GL_OES_standard_derivatives : enable

precision highp float;

varying vec2 vs_texCoord;
varying vec4 vs_color;

void main(void)
{
    const float Radius = 0.5;
    float dist = distance(vs_texCoord, vec2(0.5, 0.5));
    float feather = fwidth(dist);
    float alpha = smoothstep(Radius, Radius - feather, dist);
    gl_FragColor = vec4(vs_color.xyz, alpha * vs_color.w);
}
