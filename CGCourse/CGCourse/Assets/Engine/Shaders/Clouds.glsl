#shader vertex
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;
layout (location = 3) in vec3 geo_Tangent;
layout (location = 4) in vec3 geo_Bitangent;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4 weights;

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

out vec3 fragCoord;

void main() {
    gl_Position = ubo_Projection * ubo_View * ubo_Model * vec4(geo_Pos, 1.0);
    fragCoord = vec3(ubo_Model * vec4(geo_Pos, 1.0));
}

#shader fragment
#version 460 core

layout (std140) uniform EngineUBO {
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform vec2 iMouse;

vec2 iResolution = vec2(800, 600);
// Comment this string to see another type of clouds
//#define TWISTED
// Comment this string to make cloud less dense
#define DENSE

#define DITHERING

#define pi 3.14159265
#define R(p, a) p=cos(a)*p+sin(a)*vec2(p.y, -p.x)

// iq's noise
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
    uv = (uv+ 0.5)/256.0;
    uv.x += ubo_Time * 0.01;
	vec2 rg = textureLod( iChannel0, uv, 0.0).yx;
	return mix( rg.x, rg.y, f.z );
}

float fbm( vec3 p )
{
    return noise(p*.06125)*.75+ noise(p*.125)*.325 + noise(p*.4)*.2;
}

// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
float rand(vec2 co)
{
    return fract(sin(dot(co*0.123,vec2(12.9898,78.233))) * 43758.5453);
}

float Sphere( vec3 p, float r )
{
    return length(p)-r;
}

//==============================================================
// otaviogood's noise from https://www.shadertoy.com/view/ld2SzK
//--------------------------------------------------------------
// This spiral noise works by successively adding and rotating sin waves while increasing frequency.
// It should work the same on all computers since it's not based on a hash function like some other noises.
// It can be much faster than other noise functions if you're ok with some repetition.
const float nudge = 2.;	// size of perpendicular vector
float normalizer = 1.0 / sqrt(1.0 + nudge*nudge);	// pythagorean theorem on that perpendicular to maintain scale
float SpiralNoiseC(vec3 p)
{
    //use ubo_Time to animw
    float n = -mod(5 * 0.2,-2.); // noise amount
    //float n =  0.3f * sin(ubo_Time); // noise amount
    float iter = 2.0  + ubo_Time * 0.001;
    for (int i = 0; i < 8; i++)
    {
        // add sin and cos scaled inverse with the frequency
        n += -abs(sin(p.y*iter) + cos(p.x*iter)) / iter;	// abs for a ridged look
        // rotate by adding perpendicular and scaling down
        p.xy += vec2(p.y, -p.x) * nudge;
        p.xy *= normalizer;
        // rotate on other axis
        p.xz += vec2(p.z, -p.x) * nudge;
        p.xz *= normalizer;
        // increase the frequency
        iter *= 1.733733;
    }
    return n;
}

float SpiralNoise3D(vec3 p)
{
    float n = 0.0;
    float iter = 2.0  + ubo_Time * 0.001;
    for (int i = 0; i < 5; i++)
    {
        n += (sin(p.y*iter) + cos(p.x*iter)) / iter;
        //p.xy += vec2(p.y, -p.x) * nudge;
        //p.xy *= normalizer;
        p.xz += vec2(p.z, -p.x) * nudge;
        p.xz *= normalizer;
        iter *= 1.33733;
    }
    return n;
}

float VolumetricCloud(vec3 p)
{
    float final = Sphere(p,4.);
    #ifdef TWISTED
    float tnoise = noise(p*0.5);
    //final += tnoise * 1.75;
    final += SpiralNoiseC(p.zxy*0.3132*tnoise+333.)*3.25;
    #else
    final += SpiralNoiseC(p*0.35+333.)*3.0 + fbm(p*50.)*1.25;
    #endif
    return final;
}

float map(vec3 p) 
{
	float VolCloud = VolumetricCloud(p/0.5)*0.5; // scale
	return VolCloud;
}

bool RaySphereIntersect(vec3 org, vec3 dir, out float near, out float far)
{
	float b = dot(dir, org);
	float c = dot(org, org) - 26.;
	float delta = b*b - c;
	if(delta < 0.0) 
		return false;
	float deltasqrt = sqrt(delta);
	near = -b - deltasqrt;
	far = -b + deltasqrt;
	return far > 0.0;
}

float HGPhase( float cosAngle, float g)
{
	float g2 = g * g;
	return (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosAngle, 1.5);
}

float BeerLaw(float DenSample, float DenCoef)
{
   return exp( -DenSample * DenCoef);
}
         
float PowderEff(float DenSample, float cosAngle, float DenCoef, float PowderCoef)
{
   float Powder = 1.0 - exp(-DenSample * DenCoef * 2.0);
   Powder = clamp(Powder * PowderCoef, 0.0, 1.0);
   return mix(1.0, Powder, smoothstep(0.5, -0.5, cosAngle));
}

// Utility function that maps a value from one range to another.
float Remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
	return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}

out vec4 fragColor;
in vec3 fragCoord;

void main()
{  
    const float KEY_1 = 49.5/256.0;
	const float KEY_2 = 50.5/256.0;
	const float KEY_3 = 51.5/256.0;
    float key = 0.0;
    key += 0.7*texture(iChannel1, vec2(KEY_1,0.25)).x;
    key += 0.7*texture(iChannel1, vec2(KEY_2,0.25)).x;
    key += 0.7*texture(iChannel1, vec2(KEY_3,0.25)).x;
    
   	// ro: ray origin
	// rd: direction of the ray
	//vec3 rd = normalize(vec3((gl_FragCoord.xy-0.5*iResolution.xy)/iResolution.y, 1.));
    //Cloud pos
	vec3 rd = normalize(ubo_ViewPos - fragCoord);
	vec3 ro = vec3(0., 0., -8.+key*1.6);
    
    R(rd.yz, -pi*3.93);
    R(ro.yz, -pi*3.93);
    //R(rd.xz, iMouse.x*0.008*pi);
    //R(ro.xz, iMouse.x*0.008*pi);
   
    // ld, td: local, total density 
    // w: weighting factor
    float ld=0., td=0., w;

    // t: length of the ray
    // d: distance function
    float d=1., t=0.;
   
    // Distance threshold
    const float h = .225;
    
    //Sun pos
    vec3 sundir = normalize( vec3(-1.0,0.75,1.0) );
   
    // background sky     
    vec3 colSky = vec3(0.6,0.71,0.75) - rd.y*0.2*vec3(1.0,0.5,1.0) + 0.15*0.5;
    float sun = clamp(dot(sundir,rd), 0.0, 1.0);
    colSky += 0.25 * vec3(1.0,.6,0.1) * pow(sun, 8.0) + 2.0 * vec3(1.0,.6,0.1) * pow(sun, 3000.0);
    colSky = clamp(colSky, 0.0, 1.1);
    
    vec4 sum = vec4(0.0);
   
    // Cloud color
    const vec3 CloudBaseColor = vec3(0.52, 0.67, 0.82);
    const vec3 CloudTopColor = vec3(1.0);
   
    #ifdef DITHERING
    vec2 posd = ( fragCoord.xy / iResolution.xy );
    vec2 seed = posd + fract(ubo_Time);
    t = (1. + 0.2*rand(seed*vec2(100.)));
    #endif 
    
    float d_remaped = 0.0;

    float min_dist = 0.0, max_dist = 0.0;

    if(RaySphereIntersect(ro, rd, min_dist, max_dist))
    {
       
	t = min_dist * step(t, min_dist);    
    
    // rm loop
    for (int i=0; i<64; i++)
    {
        vec3 pos = ro + t*rd;
       
        // Loop break conditions.
        if(td>0.9875 || d<0.0006*t || t>12. || t>max_dist || sum.a > 0.99) break;
       
        // evaluate distance function
        d = map(pos);
      
        d_remaped = Remap(max(d,-2.), -2., h, h, 1.0);

        // check whether we are close enough
        if (d<h) 
        {
            // compute local density and weighting factor 
            ld = h - (1.0 - d_remaped);
          
            w = (1. - td) * ld;   
     
            // accumulate density
            #ifdef DENSE
            td += w + 1./200.;
            #else
            td += w * 1./200.;
            #endif
			
            const float dShiftStep = 0.2; 
            float dShift = map(pos + dShiftStep * sundir);
            float fld = clamp((ld - (h - max(dShift, 0.0))) / (dShiftStep * 2.), 0.0, 1.0);
         
            vec3 lin = mix(CloudTopColor, CloudBaseColor, -fld) * .85;
         
            // this part based on "Real-Time Volumetric Cloudscapes" by Andrew Schneider
            const float FwdSctG = 0.2;
            const float BckwdSctG = -0.02;
            const float HGCoef = .1;
            const float DenCoef = .75;
            const float PowderCoef = 1.3;
            
            float cosAngle = dot(rd, sundir);
            float FwdSctHG = HGPhase(cosAngle, FwdSctG);
            float BckwdSctHG = HGPhase(cosAngle, BckwdSctG);
            float TotalHGPhase = (HGCoef * FwdSctHG) + ((1. - HGCoef) * BckwdSctHG);
			
            #ifdef DENSE
            vec4 col = vec4(lin * TotalHGPhase * BeerLaw(fld*ld, DenCoef) * PowderEff(exp(-d), cosAngle, DenCoef, PowderCoef) + colSky * 0.05, d_remaped);
            #else
            vec4 col = vec4(lin * TotalHGPhase * BeerLaw(fld*ld, DenCoef) * PowderEff(exp(-d), cosAngle, DenCoef, PowderCoef) + colSky * 0.05, exp(-d_remaped));
            #endif
            
            // front to back blending
            #ifdef DENSE
            col.a *= 0.185;
            #else
            col.a *= 0.175;
            #endif
            col.rgb *= col.a;
            sum += col*(1.0-sum.a);
        }
      
        td += 1./150.; // 1./80.;
       
        // enforce minimum stepsize
        d = max(d, 0.1);
      
        #ifdef DITHERING
        // add in noise to reduce banding and create fuzz
        posd.y*=120.;
        posd.x*=280.;
        d=abs(d)*(.8+0.08*texture(iChannel2,vec2(posd.y,-posd.x+0.5*sin(4.*ubo_Time+posd.y*4.0))).r);
        #endif 
     
        // step forward
        t += max(d * 0.19, 0.02);
    }
 
    sum = clamp( sum, 0.0, 1.0 );
    
    }
        
    sum.xyz = colSky * (1.0-sum.w) + sum.xyz;
    
    // small color adjustment
    sum.xyz *= 1. / exp( ld * 0.05 ) * .85;
    sum.xyz = sum.xyz*sum.xyz*(3.0-2.0*clamp(sum.xyz,0.0,1.0));
   
    fragColor = vec4(sum.xyz,1.0);
}
