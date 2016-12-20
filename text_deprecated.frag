#ifdef GL_ES
    precision mediump float;
#endif

uniform sampler2D texture;

varying vec2  v_texcoord;
varying vec4  v_color;
varying float v_channel;

float sampleChnl(vec2 texcoord, float channel){
    vec4 val = texture2D(texture, vec2(texcoord.x, 1.0 - texcoord.y));
    if(channel == 1){
        return val.b;
    }
    else if(channel == 2){
        return val.g;
    }
    else if(channel == 3){
        return val.r;
    }
    else{
        return val.a;
    }
}
void main(void)
{
    gl_FragColor = v_color * pow(sampleChnl(v_texcoord, v_channel), 0.5);
}
