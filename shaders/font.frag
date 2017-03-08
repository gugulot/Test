#version 100

precision mediump float;
uniform sampler2D u_s2dTexture;
varying vec2 v_v2TexCoord;
varying vec4 v_v4FontColor;
void main()
{
    vec4 v4Texel = texture2D(u_s2dTexture, v_v2TexCoord);
    gl_FragColor = v_v4FontColor * v4Texel;
}
