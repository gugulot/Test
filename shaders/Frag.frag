precision lowp float;

uniform float iGlobalTime;
vec2 iResolution = vec2(1920,1080);
vec3 iMouse = vec3(1.0,1.0,1.0);


varying vec3 vv3colour;

void main()
{

    vec2 uv = ((30.0 + iGlobalTime) / 3.0) * ((gl_FragCoord.xy - (iResolution.xy / 2.0)) / iResolution.y);
    float value = (uv.x * sign(uv.y) + uv.y * sign(uv.x)) * sin(uv.x) * sin(uv.y);
    float color = sin(value) * 3.0;
    color = color * sign(uv.x) * sign(uv.y); // revision 2
    float low = abs(color);
    float med = abs(color) - 1.0;
    float high = abs(color) - 2.0;
    if(color > 0.5) {
         gl_FragColor = vec4(high, med, high,1.0) * vec4(vv3colour, 1.0);
    } else {
      gl_FragColor = vec4(med, high, med,1.0) * vec4(vv3colour, 1.0);
    }



    //gl_FragColor = vec4(vv3colour, 1.0);
}
 
