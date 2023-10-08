#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;
uniform bool u_UseHDR;
uniform float u_Exposure;
uniform float u_Gamma;

//uniform mat4 camera_transform_matrix;
//uniform mat4 shadow_map_transform_mat;
//uniform sampler2D shadow_map_depth_tex;
//uniform sampler2D depth_tex;

//god rays
//uniform vec3 sunPos;

//layout (std140) uniform EngineUBO
//{
//    mat4    ubo_Model;
//    mat4    ubo_View;
//    mat4    ubo_Projection;
//    vec3    ubo_ViewPos;
//    float   ubo_Time;
//    vec2    ubo_ViewportSize;
//};

void main() {
    /*
    //god rays
    int NUM_SAMPLES = 80;
	vec2 tc = TexCoords.xy;
	vec2 deltatexCoord = (tc - (sunPos.xy*0.5 + 0.5));
	deltatexCoord *= 1.0/ float(NUM_SAMPLES);
	float illuminationDecay = 1.0f;

	vec4 godRayColor = texture(bloomBlur , tc.xy)*0.4;
	for(int i = 0 ; i< NUM_SAMPLES ; i++) {
		tc-= deltatexCoord;
		vec4 samp = texture(bloomBlur , tc )*0.4;
		samp *= illuminationDecay*weight;
		godRayColor += samp;
		illuminationDecay *= decay;
	}
	vec3 realColor = hdrColor;
	hdrColor = godRayColor.rgb * 0.3f + realColor;
    */
    /*
    int Samples = 128;
    float Intensity = 0.125;
    float Decay = 0.96875;
    vec2 _TexCoord = TexCoords.xy;
    vec2 Direction = vec2(0.5) - _TexCoord;
    Direction /= Samples;
    vec3 Color = texture2D(bloomBlur, _TexCoord).rgb;
    
    for(int Sample = 0; Sample < Samples; Sample++) {
        Color += texture2D(bloomBlur, _TexCoord).rgb * Intensity;
        Intensity *= Decay;
        _TexCoord += Direction;
    }
    hdrColor = Color.rgb * 0.3f + hdrColor;
    */

    vec3 color = texture(u_Scene, TexCoords).rgb;
    /* volumetric light
    float aspect_ratio = 800.0/600.0;
    float fov_angle = 45.0f;

    float point_x = (2*TexCoords.x-1)*aspect_ratio*tan(fov_angle/2);
    float point_y = (2*TexCoords.y-1)*tan(fov_angle/2);

    vec3 vector_pixel = vec3(point_x, point_y, -1);

    //find world pos of the pixel and the camera origin, then we have the vector of the ray in world
    vec3 vector_pixel_world = vec3(inverse(camera_transform_matrix)*vec4(vector_pixel, 1.0));
    vec3 vector_origin_world = vec3(inverse(camera_transform_matrix)*vec4(0.0, 0.0, 0.0, 1.0));
    vec3 vector_dir = vector_pixel_world-vector_origin_world;
    
    mat4 PV_mat = ubo_Projection * camera_transform_matrix;
    int max_iterations = 50;
    vec3 cur_vec = vector_pixel_world;
    for(int i = 0; i < max_iterations; i++){
       vec4 end_vec = PV_mat*vec4(cur_vec, 1.0);

       //find corresponding pixel of the 3d world point on the shadow depth map
       vec4 shadow_coord_tex = shadow_map_transform_mat*vec4(cur_vec, 1.0);

       float fog = 0;
       float fog_altitude = 5;

       //have some fog effect at low altitude
       float fog_calc = 1;
       if(cur_vec.y > fog_altitude-1 && cur_vec.y < fog_altitude){
          fog_calc = fog_altitude-cur_vec.y;
       }
       else if(cur_vec.y >= fog_altitude){
          fog_calc = 0;
       }

       if(fog_calc > 0){
          //bias for the shadow map, helps against aliasing
          float BIAS = 0.002;
          //bias for the camera depth, avoid seeing lights "through" the objects
          float BIAS_DEPTH = 0.02;
          //check that the end of the vector is not further than the depth map (behind an object)
          if(end_vec.z/end_vec.w+BIAS_DEPTH < texture(depth_tex, TexCoords).r){
             //check that the ray from the end vector to the light has no obstruction with the help of the shadow depth map
             if ( texture( shadow_map_depth_tex, (shadow_coord_tex.xy/shadow_coord_tex.w) ).x > shadow_coord_tex.z/shadow_coord_tex.w-BIAS){
                //if ray not obstructed from cam to end_vec and then to light, then add a bit of colour (vol light)
                color += fog_calc*vec3(0.016);
             }
          }
       }
       //increment for the ray tracing
       cur_vec = cur_vec + 0.15*vector_dir;
    }*/

    //vec3 color = texture(u_Scene, TexCoords).rgb;
    if(u_UseHDR) {
        //vec3 result = color / (color + vec3(1.0));
		
        // ����������
        vec3 result = vec3(1.0) - exp(-color * u_Exposure);
		
        // ������ �������� �����-���������
        result = pow(result, vec3(1.0 / u_Gamma));

        FragColor = vec4(result, 1.0);
    }
    else {
        vec3 result = pow(color, vec3(1.0 / u_Gamma));
        FragColor = vec4(result, 1.0);
    }
}
