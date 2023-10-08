#shader vertex
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D u_Scene;
uniform mat4 u_CameraTransformMatrix;
uniform mat4 u_ShadowMapTransformMatrix;
uniform sampler2D u_ShadowMapDepthTex;
uniform sampler2D u_DepthTex;

//god rays
//uniform vec3 sunPos;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
    vec2    ubo_ViewportSize;
};

void main() {
    vec3 color = texture(u_Scene, TexCoords).rgb;
    // volumetric light
    float aspect_ratio = 800.0/600.0;
    float fov_angle = 45.0f;

    float point_x = (2*TexCoords.x-1)*aspect_ratio*tan(fov_angle/2);
    float point_y = (2*TexCoords.y-1)*tan(fov_angle/2);

    vec3 vector_pixel = vec3(point_x, point_y, -1);

    //find world pos of the pixel and the camera origin, then we have the vector of the ray in world
    vec3 vector_pixel_world = vec3(inverse(u_CameraTransformMatrix)*vec4(vector_pixel, 1.0));
    vec3 vector_origin_world = vec3(inverse(u_CameraTransformMatrix)*vec4(0.0, 0.0, 0.0, 1.0));
    vec3 vector_dir = vector_pixel_world-vector_origin_world;
    
    mat4 PV_mat = ubo_Projection * u_CameraTransformMatrix;
    int max_iterations = 300;
    vec3 cur_vec = vector_pixel_world;
    for(int i = 0; i < max_iterations; i++){
       vec4 end_vec = PV_mat*vec4(cur_vec, 1.0);

       //find corresponding pixel of the 3d world point on the shadow depth map
       vec4 shadow_coord_tex = u_ShadowMapTransformMatrix*vec4(cur_vec, 1.0);

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
          float BIAS = 0.45;
          //bias for the camera depth, avoid seeing lights "through" the objects
          float BIAS_DEPTH = -0.35;
          //check that the end of the vector is not further than the depth map (behind an object)
          if(end_vec.z/end_vec.w+BIAS_DEPTH < texture(u_DepthTex, TexCoords).r){
             //check that the ray from the end vector to the light has no obstruction with the help of the shadow depth map
             if ( texture( u_ShadowMapDepthTex, (shadow_coord_tex.xy/shadow_coord_tex.w) ).x > shadow_coord_tex.z/shadow_coord_tex.w-BIAS){
                //if ray not obstructed from cam to end_vec and then to light, then add a bit of colour (vol light)
                 //color = vec3(1,0,0);
                 color += fog_calc * vec3(0.016);
             }
          }
       }
       //increment for the ray tracing
       cur_vec = cur_vec + 0.15*vector_dir;
    }

    //color = texture(u_DepthTex, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
}
