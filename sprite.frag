#ifdef GL_ES
    precision mediump float;
#endif

uniform sampler2D texture;

varying vec2  v_texcoord;
varying vec2  v_scale;
varying vec2  v_scale_lo;
varying vec2  v_scale_hi;
varying float v_id;
varying vec4  v_hilight;


void main()
{
    vec2 texcoord = v_texcoord;
    vec4 color;

    texcoord.s = (texcoord.s - v_scale_lo.s) * v_scale.s;
    texcoord.t = (texcoord.t - v_scale_lo.t) * v_scale.t;

    texcoord.s = texcoord.s + v_scale_lo.s;
    texcoord.t = texcoord.t + v_scale_lo.t;

    color = texture2D(texture, vec2(texcoord.s, 1.0 - texcoord.t)) + v_hilight;
    color = clamp(color, 0.0, 1.0) * color.a;

    gl_FragData[0] = color;
    if(color.a > 0){
        gl_FragData[1] = vec4(v_id,0.0,0.0,1.0);
    }
    else{
        gl_FragData[1] = vec4(0.0,0.0,0.0,0.0);
    }
}
