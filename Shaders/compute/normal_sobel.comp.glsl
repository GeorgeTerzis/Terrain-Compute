#version 460 core
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D height;
layout(rgba32f, binding = 1) uniform image2D sobel;

vec3 norm_sobel(image2D h,ivec2 uv,float str){
  if(str > 1){str = 1;}
  else if(str < 0){str = 0;}

  const float t  = imageLoad(h,uv + ivec2(0,-1)).r;
  const float tl = imageLoad(h,uv + ivec2(-1,-1)).r;
  const float l  = imageLoad(h,uv + ivec2(-1,0)).r;
  const float b  = imageLoad(h,uv + ivec2(0,1)).r;
  const float bl = imageLoad(h,uv + ivec2(-1,1)).r;
  const float tr = imageLoad(h,uv + ivec2(1,-1)).r;
  const float r  = imageLoad(h,uv + ivec2(1,0)).r;
  const float br = imageLoad(h,uv + ivec2(1,1)).r;
  const float dx = (tr + 2*r + br) - (tl + 2*l + bl);
  const float dy = (bl + 2*b + br) - (tl + 2*t + tr);
  const float dz = str;
  return normalize(vec3(dx,dy,dz));
}
void main(){
  vec4 outv = vec4(0.0, 0.0, 0.0, 1.0);
  ivec2 tcoord = ivec2(gl_GlobalInvocationID.xy);

  vec2 uv = vec2(
    float(tcoord.x)/(gl_NumWorkGroups.x),
    float(tcoord.y)/(gl_NumWorkGroups.y)
  );
  vec3 norm = norm_sobel(height,tcoord,1);
  imageStore(sobel, tcoord, vec4(norm.xyz,1.0));
}

