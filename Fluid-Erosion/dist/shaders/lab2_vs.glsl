#version 420            
uniform mat4 PVM;
uniform float sys_time;
uniform float SineScale;

in vec3 pos_attrib; //this variable holds the position of mesh vertices
in vec2 tex_coord_attrib;
in vec3 normal_attrib;  

out vec2 tex_coord; 
out vec3 normal_coord;
out vec3 pos_coord;
out vec3 frag_coord;

void main(void)
{
	gl_Position = PVM*vec4(pos_attrib, 1.0); //transform vertices and send result into pipeline
	pos_coord = vec3(gl_Position.xyz);
	tex_coord = vec2(tex_coord_attrib.x+SineScale*sin(pos_attrib.y*sys_time), tex_coord_attrib.y+SineScale*sin(pos_attrib.x*sys_time)); //send tex_coord to fragment shader
	vec4 norm = PVM * vec4(normal_attrib, 1.0);
	normal_coord = normalize(vec3(norm.xyz));
	frag_coord = vec3((PVM * vec4(pos_attrib, 1.0)).xyz);
}