uniform mat4 mvp_matrix;

attribute vec4 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;
flat varying int instanceNum;

void main(void)
{
    gl_Position = mvp_matrix * a_position;
    v_texcoord  = a_texcoord;
    instanceNum = int(gl_VertexID / 4.0) * 2;
}
