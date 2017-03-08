precision mediump float;
varying vec3 vv3colour;
uniform float iGlobalTime;
vec2 iResolution = vec2(400,600);
vec2 iMouse = vec2(1.0,1.0);

//	gl_FragColor = vec4(vv3colour,1.0);


#define SKY_MULT 0.98
#define GRASS_CONTRAST (5.5)
#define GRASS_SHADOW_CONTRAST (3.6)
#define GRASS_CONTRAST_INV (1.0 / GRASS_CONTRAST)
#define GRASS_SHADOW_CONTRAST_INV (1.0 / GRASS_SHADOW_CONTRAST)
#define GRASS_WIDTH (0.01)
#define GRASS_WIDTH_INV (1.0 / GRASS_WIDTH)
#define GRASS_BENDING_AMP 5.0
#define GRASS_BENDING_PERIOD 0.05
#define GP2 0.4317

vec4 get_grass(vec2 uv, float seed)
{
    if (uv.y > 1.0)
        return vec4(0.0);
    else if (uv.y < 0.0)
        return vec4(1.0);
    else 
    {        
        float seed2 = seed * 11.2;
        float seed3 = seed * 3.621 - 43.32;
        float bending = abs(0.5 - fract(seed2 + uv.x * GRASS_BENDING_PERIOD * GRASS_WIDTH_INV)) - 0.25;
        uv.x += GRASS_WIDTH * GRASS_BENDING_AMP * 4.0 * uv.y * uv.y * uv.y * bending;
        float shadowX = uv.x - uv.y * (0.25 + fract(seed2) * 0.22);
        float top = 4.0 * abs(0.5 - fract(seed + uv.x * GRASS_WIDTH_INV)) * abs(0.5 - fract(seed3 + uv.x * GRASS_WIDTH_INV * GP2));
        float topB= 3.0 * abs(0.5 - fract(seed + (uv.x + 0.005) * GRASS_WIDTH_INV)) * abs(0.5 - fract(seed3 + (uv.x - 0.009) * GRASS_WIDTH_INV * GP2));
        float topR= 3.0 * abs(0.5 - fract(seed + (uv.x + 0.006 * (1.0 - uv.y)) * GRASS_WIDTH_INV)) * abs(0.5 - fract(seed3 + (uv.x + 0.009 * (1.0 - uv.y)) * GRASS_WIDTH_INV * GP2));
        float topS= 2.5 * abs(0.5 - fract(seed + 0.7 * (shadowX + 0.31) * GRASS_WIDTH_INV)) * abs(0.5 - fract(seed3 +  0.7 * (shadowX + 0.161) * GRASS_WIDTH_INV * GP2));
        uv.y = uv.y * uv.y;
        float alpha = GRASS_CONTRAST * (uv.y - (1.0 - GRASS_CONTRAST_INV) * top);
        float bright = GRASS_CONTRAST * (uv.y - topB);
        float bright2 = GRASS_CONTRAST * (uv.y - topR);
        float shadow = GRASS_SHADOW_CONTRAST * (uv.y - (1.0 - GRASS_SHADOW_CONTRAST_INV) * topS);
        return clamp(vec4(1.0 - alpha, bright, 1.0 - shadow, bright2), 0.0, 1.0);
    }
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.y;
    vec3 c = mix(vec3(0.53, 0.63, 0.78), vec3(0.42, 0.52, 0.65), uv.y - uv.x * 0.5) * SKY_MULT;
    float elevation = iMouse.y / iResolution.y;
    uv.y += 0.3 - elevation * 0.8;
    uv.y *= 1.8;
    float k = 1.0;
    vec3 grassColor = vec3(0.4, 0.9, 0.1);
    vec3 grassBackColor = grassColor * (0.25 + elevation * 0.2);
    vec3 grassColorR = vec3(0.65, 0.7, 0.3);
    vec3 grassShadow = grassColor * vec3(0.15, 0.2, 0.9);
    
    float pos = iGlobalTime * 3.0;
    float iPos = floor(pos);
    float fPos = fract(pos);
    
    uv.x += sin(pos * 0.3) * 0.4;
    
    for (int i = 10; i >= 0; i--)
    {
        float dist = (float(i) - fPos) / 10.0;
        vec2 uv2 = uv;
        uv2 *= 0.15 + dist * 1.4;
        uv2.y += elevation + 0.45 - dist * (0.5 + elevation);
        vec4 grass = get_grass(uv2, fract((iPos + float(i)) * 43.2423));
        vec3 color = mix(grassBackColor, grassColor, grass.y);
        color = mix(color, grassColorR, grass.w);
        color = mix(color, grassShadow, grass.z);
        if (i == 0)
            grass.x *= smoothstep(0.0, 1.0, 1.0 - fPos);
        if (i == 10)
            grass.x *= fPos;
        c = mix(c, color, grass.x);
    }
       
	gl_FragColor = vec4(c, 1.0) + vec4(vv3colour,1.0);
}
