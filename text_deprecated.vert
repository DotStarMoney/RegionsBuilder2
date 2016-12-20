uniform mat4 mvp_matrix;

attribute vec4 a_position;
attribute vec2 a_texcoord;
attribute vec4 a_color;
attribute vec4 a_channel;

varying vec2  v_texcoord;
varying vec4  v_color;
varying float v_channel;

void main(void)
{
    gl_Position = mvp_matrix * a_position;
    v_texcoord = a_texcoord;
    v_color    = a_color;
    v_channel  = a_channel.x;
}
