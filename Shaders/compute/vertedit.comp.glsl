#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0)uniform sampler2D height;

uniform float time;
uniform int px;
uniform int py;

layout(std430, binding = 0) buffer verts{
    vec4 data[];
};
float rand1d(vec3 p) {
    return fract(sin(p.x*1020.+p.y*251.+p.z*21.0)*562447.);
}
float rand1d(vec2 p) {
    return fract(sin(p.x*120.+p.y*261.)*5062447.);
}
float rand2d(vec2 p) {
    return (fract(sin(p.x*1020.+p.y*251.)*562447.));
}
 vec4 noised(vec3 x) {
    vec3 p = floor(x);
    vec3 w = smoothstep(0.0,1.0,fract(x));

    vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    vec3 du = 30.0*w*w*(w*(w-2.0)+1.0);

    float a = rand1d( p+vec3(0,0,0) );
    float b = rand1d( p+vec3(1,0,0) );
    float c = rand1d( p+vec3(0,1,0) );
    float d = rand1d( p+vec3(1,1,0) );
    float e = rand1d( p+vec3(0,0,1) );
    float f = rand1d( p+vec3(1,0,1) );
    float g = rand1d( p+vec3(0,1,1) );
    float h = rand1d( p+vec3(1,1,1) );

    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    return vec4( (k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z),
                 2.0* du * vec3( k1 + k4*u.y + k6*u.z + k7*u.y*u.z,
                                 k2 + k5*u.z + k4*u.x + k7*u.z*u.x,
                                 k3 + k6*u.x + k5*u.y + k7*u.x*u.y ) );
  }


//rotation
//I need to find the highest value
//and the lowest value
//scale the whole thing to those min and max
const mat2 m = mat2(0.8,-0.6,0.6,0.8);
float fbm(vec2 p){
  float a = 0.0;
  float b = 1.0;
  vec2  d = vec2(0.0);
  for(int i=0; i < 32; i++){
    vec3 n= noised(vec3(m*p,1)).xyz;
    d +=n.yz;
    a +=b*n.x/(1.0+dot(d,d));
    b *=0.5;
    p=m*p*2.;
  }
  return a;
}

vec2 grad(vec2 pos){
  float twoPi = 6.2885;
  float angle = rand1d(pos)*twoPi;
  return vec2(cos(angle), sin(angle));
}

float pnoise(vec2 uv){
  const vec2 center = uv + vec2(0.5);
  const vec2 id = floor(uv);
  const vec2 fract = smoothstep(0.0,1.0,fract(uv));

  const vec2 bl = id;
  const vec2 br = id+vec2(1,0);
  const vec2 tl = id+vec2(0,1);
  const vec2 tr = id+vec2(1,1);

  const float dbl = dot(center - bl,grad(bl));
  const float dbr = dot(center - br,grad(br));
  const float dtl = dot(center - tl,grad(tl));
  const float dtr = dot(center - tr,grad(tr));

  const float b = mix(dbl,dbr,fract.x);
  const float t = mix(dtl,dtr,fract.x);

  const float tb = mix(b,t,fract.y);
  return tb;
}
float pfbm(vec2 uv){
  float str = 1;
  float str_decay = 0.5;
  float scale = 2.0f;
  float val = 0.0f;
  for(int i = 0; i < 10; i++){
    val += str*pnoise(uv);
    uv *= vec2(scale);
    str *= str_decay;
  }

  return val;
}

uniform int meshres;

void main(){
  uint index = gl_GlobalInvocationID.x;
  vec2 uv = vec2(data[index].x + px - 1,data[index].z + py - 1);
//  float h = pow(texture(height,0.7*(uv + 0.6)).r*0.1 + 1,1) - 1;
  float h = 0;
  h += fbm((uv*4)*0.3)*0.1;
  h += smoothstep(0.7,0.2,abs(pfbm(uv) - 0.4))*0.1;
  h *=  clamp(1,0,smoothstep(1,0,dot(uv,uv) - 1));

  data[index].y = h;
  data[index].x += px - 1;
  data[index].z += py - 1;
}
