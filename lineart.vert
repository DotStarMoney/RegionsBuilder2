uniform mat4 mvp_matrix;

attribute vec4 a_position;
attribute vec4 a_color;

varying vec4 v_color;

void main(void)
{
    gl_Position = mvp_matrix * a_position;
    v_color = a_color;
}
