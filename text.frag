#ifdef GL_ES
    precision mediump float;
#endif

uniform sampler2D     texture;
uniform samplerBuffer instance_tbo;

varying vec2     v_texcoord;
flat varying int instanceNum;

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
//r g b a
void main(void)
{
    vec4 chnl_uu3 = texelFetchBuffer(instance_tbo, instanceNum + 0);
    vec4 color    = texelFetchBuffer(instance_tbo, instanceNum + 1);

    gl_FragColor = color * pow(sampleChnl(v_texcoord, chnl_uu3.x), 0.5);
}
