#version 460 core

out vec4 fragcol;
in vec4 opos;
in vec2 tcoords;
in vec2 cell;
layout(binding = 0)uniform sampler2D height;
//layout(binding = 1)uniform sampler2D tnorm;
layout(binding = 3)uniform sampler2D ground;
layout(binding = 4)uniform sampler2D rock;
uniform float time;
uniform int index;

vec3 calcssnorm(vec3 p) {
    vec3 dx = dFdx(p);
    vec3 dy = dFdy(p);
    return normalize(cross(dx, dy));
}


vec3 lpos = vec3(0,2,0);


float slope(vec3 n){
  return (n.x*n.x) + (n.z*n.z)/(n.y*n.y);
}


void main(){
  lpos += vec3(sin(time/5),0,cos(time/5));
  vec3 norm = calcssnorm(opos.xyz);
  vec3 ldir = normalize(lpos - vec3(opos.xyz));  
  float diff = clamp(1,0,max(dot(norm, ldir), 0.0)*clamp(1,0,0.4*(8 - distance(opos.xyz,lpos))));  
  
  vec3 gcol = texture(ground,tcoords*5).rgb;
  vec3 rcol = texture(rock,tcoords*3).rgb;
 
  vec3 col = mix(vec3(0.2,0.2,0.5),vec3(0.5,0.2,0.2),clamp(1,0,2*slope(norm)));
  col = mix(gcol,rcol,clamp(1,0,2*slope(norm)));

  float m1 = 1 - clamp(1,0,slope(norm));
  // m1 *= 0.1;
  m1 = clamp(1,0,smoothstep(0.2,1,opos.y)*pow(m1,2)*100);
  col = mix(col,vec3(1),m1*0*abs(sin(time*2)));
  // float h = sin(time*20)*texture(height,tcoords).r;
//  col = vec3(tcoords,0);
//  if(tcoords.x > 1&& tcoords.y > 1)
//    fragcol = vec4(vec3(1,0,1),1);
//  else if(tcoords.x < -1&&tcoords.y < -1)
//    fragcol = vec4(vec3(0,1,1),1);
//  else
    
    // col = vec3(cell/3,0);
    // fragcol = vec4(vec3(col), 1.0f);
    fragcol = vec4(vec3(col*diff), 1.0f);
}
