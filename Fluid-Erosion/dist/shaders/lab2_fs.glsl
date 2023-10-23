#version 420
layout (std140, binding = 1) uniform TextureBlock{
	bool showTexture;
}textures;
layout(std140, binding = 2) uniform ColorBlock{
	vec3 Ka_color;
	vec3 Kd_color;
	vec3 Ks_color;
}colors;
layout(std140, binding = 3) uniform LightBlock{
	vec3 position;
	vec3 Ka_color;
	vec3 Kd_color;
	vec3 Ks_color;
}lights;

uniform sampler2D diffuse_tex;
uniform vec3 sys_cameraPos;
uniform float sys_shininess;

in vec2 tex_coord; 
in vec3 normal_coord;
in vec3 pos_coord;
in vec3 frag_coord;

out vec4 fragcolor; //the output color for this fragment    

void main(void)
{   
	vec3 ambient = vec3(colors.Ka_color.x * lights.Ka_color.x, colors.Ka_color.y * lights.Ka_color.y, colors.Ka_color.z * lights.Ka_color.z);

	if (textures.showTexture) {
		vec3 col = texture(diffuse_tex, tex_coord).xyz;
		ambient = vec3(col.x * lights.Ka_color.x, col.y * lights.Ka_color.y, col.z * lights.Ka_color.z);
	}

	float att = 1.0f / length(vec3(pos_coord.x - lights.position.x, pos_coord.y - lights.position.y, pos_coord.z - lights.position.z));

	float diffusedot = max(0, dot(normal_coord, lights.position));

	vec3 diffuse = vec3(colors.Kd_color.x * lights.Kd_color.x, colors.Kd_color.y * lights.Kd_color.y, colors.Kd_color.z * lights.Kd_color.z) * att * diffusedot;

	if (textures.showTexture) {
		vec3 col = texture(diffuse_tex, tex_coord).xyz;
		diffuse = vec3(col.x * lights.Kd_color.x, col.y * lights.Kd_color.y, col.z * lights.Kd_color.z) * att * diffusedot;
	}

	vec3 viewDir = normalize(sys_cameraPos - frag_coord.xyz);
	vec3 reflected = reflect(-1.0 * lights.position, normal_coord);

	float specdot = pow(max(0, dot(normalize(reflected), viewDir)), sys_shininess);

	vec3 specular = vec3(colors.Ks_color.x * lights.Ks_color.x, colors.Ks_color.y * lights.Ks_color.y, colors.Ks_color.z * lights.Ks_color.z) * att * specdot;

	fragcolor = vec4(specular, 1.0f);
}




















