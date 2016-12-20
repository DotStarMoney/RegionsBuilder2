#ifdef GL_ES
    precision mediump float;
#endif

uniform sampler2D texture;

varying vec2  v_texcoord;

void main()
{
    gl_FragColor = texture2D(texture, vec2(v_texcoord.s, 1.0 - v_texcoord.t));
}
