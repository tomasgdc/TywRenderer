#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


in VS_OUT
{
	mat3 TBN;
	vec3 ws_coords;
    vec2 texcoord;
} fs_in;

layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerDiffuse;
layout (binding = 3) uniform sampler2D samplerSpecular;

//Position
layout (location = 0) out vec4 color0;

//Normal and Diffuse
layout (location = 1) out uvec4 color1;

//TODO: Deal with specular
void main() 
{	
	
	//Get correct coords
	vec2 flipped_texcoord = vec2(fs_in.texcoord.x, 1.0 - fs_in.texcoord.y);
	vec3 diffuseTexture = texture(samplerDiffuse, flipped_texcoord).rgb;
	vec3 normalTexture  = normalize( (1.0 - texture(samplerNormal, flipped_texcoord).rgb) * 2.0);
	//vec3 specularTexture = texture(samplerSpecular, flipped_texcoord).rgb;
	
	//Calculate normalTexture coords
	normalTexture = fs_in.TBN * normalTexture;

	vec4 outvec0 = vec4(0);
    uvec4 outvec1 = uvec4(0);

	//Position
	outvec0.xyz = fs_in.ws_coords;
	outvec0.w = 60.0;

	

	//Normal and diffuse packed in single texture
    outvec1.x = packHalf2x16(diffuseTexture.xy);
    outvec1.y = packHalf2x16(vec2(diffuseTexture.z, normalTexture.x));
	outvec1.z = packHalf2x16(normalTexture.yz);


    color0 = outvec0;
    color1 = outvec1;
}