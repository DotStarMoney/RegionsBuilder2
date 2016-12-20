
uniform mat4 mvp_matrix;


attribute vec4  a_position;
attribute vec2  a_texcoord;
attribute vec2  a_scale;
attribute vec2  a_scale_lo;
attribute vec2  a_scale_hi;
attribute float a_angle;
attribute vec4  a_origin;
attribute float a_id;
attribute vec4  a_hilight;

varying vec2  v_texcoord;
varying vec2  v_scale;
varying vec2  v_scale_lo;
varying vec2  v_scale_hi;
varying float v_id;
varying vec4  v_hilight;

void main() {

    v_texcoord = a_texcoord;
    v_scale    = a_scale;
    v_scale_lo = a_scale_lo;
    v_scale_hi = a_scale_hi;
    v_id       = a_id;
    v_hilight  = a_hilight;

    vec4 pos = a_position - a_origin;
    float sinA = sin(a_angle);
    float cosA = cos(a_angle);
    vec2 newxy;

    newxy.x = pos.x * cosA - pos.y * sinA;
    newxy.y = pos.y * cosA + pos.x * sinA;

    pos.x = newxy.x;
    pos.y = newxy.y;

    gl_Position = mvp_matrix * (pos + a_origin);

}
