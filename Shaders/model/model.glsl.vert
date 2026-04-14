//#version 460 core
//layout (location = 0) in float vpos;
//
////layout(binding = 0,std430) readonly buffer ssbo1 {
////    vec4 data[];
////};
//
//uniform vec3 mpos;
//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 proj;
//uniform float time;
//uniform int index;
//uniform int sindex;
//uniform int planeres;
//layout(binding = 0)uniform sampler2D height;
////layout(binding = 1)uniform sampler2D tnorm;
//
//out vec4 opos;
//out vec2 tcoords;
//out vec2 cell;
//
//int cell_line = 3;
//float cell_off; 
//
//void main(){
//
//    cell_off = float(cell_line)/2;
//    cell = vec2((float(index%cell_line)),(float(index/cell_line)));
//
//    float px = ((floor(gl_VertexID/2)/float(planeres) - 0.5) * 2*index);
//    float pz = ((((gl_VertexID%2)/float(planeres)) + sindex/float(planeres) - 0.5) * 2*index);
//    tcoords = vec2(px,pz)/18  + 0.5;
//    float py = texture(height,tcoords).r;
//    const vec3 position = vec3(px,py,pz);
//
//
//
//    vec4 p = vec4(position.xyz*0.3,1);
//    opos = model*p;
//    gl_Position = proj*view*model*p;
//}



#version 460 core
layout (location = 0) in float vpos;

//layout(binding = 0,std430) readonly buffer ssbo1 {
//    vec4 data[];
//};

uniform vec3 mpos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float time;
uniform int index;
uniform int sindex;
uniform int planeres;
layout(binding = 0)uniform sampler2D height;
//layout(binding = 1)uniform sampler2D tnorm;

out vec4 opos;
out vec2 tcoords;
out vec2 cell;

int cell_line = 3;
float cell_off; 
int level;
void main(){
    //tcoords = 0.5 + data[gl_VertexID].xz * 0.5;
    //vec4 p = vec4(data[gl_VertexID].xyz*3,1.0);
    //opos = model*p;
    
    cell_off = float(cell_line)/2;
    cell = vec2((float(index%cell_line)),(float(index/cell_line)));
    float px = ((floor(gl_VertexID/2)/float(planeres)) + cell.x) -cell_off;
    float pz = (((gl_VertexID%2)/float(planeres)) + sindex/float(planeres) + cell.y) -cell_off;
    tcoords = (vec2(px,pz) + cell_off)/cell_line;
    float py = texture(height,tcoords).r;
    const vec3 position = vec3(px, py, pz);

    vec4 p = vec4(position.xyz,1);
    opos = model*p;



    gl_Position = proj*view*model*p;
}
