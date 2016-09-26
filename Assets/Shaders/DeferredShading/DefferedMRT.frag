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

//Normal, Diffuse, Specular
layout (location = 1) out vec4 color1;


// c_precision of 128 fits within 7 base-10 digits
const float c_precision = 128.0;
const float c_precisionp1 = c_precision + 1.0;
 
//param color normalized RGB value
//returns 3-component encoded float
float color2float(vec3 color) 
{
    color = clamp(color, 0.0, 1.0);
    return floor(color.r * c_precision + 0.5) 
        + floor(color.b * c_precision + 0.5) * c_precisionp1
        + floor(color.g * c_precision + 0.5) * c_precisionp1 * c_precisionp1;
}
 

void main() 
{	
	
	//Get correct coords
	vec2 flipped_texcoord = vec2(fs_in.texcoord.x, 1.0 - fs_in.texcoord.y);
	vec3 diffuseTexture = texture(samplerDiffuse, flipped_texcoord).rgb;
	vec3 normalTexture  = normalize( (1.0 - texture(samplerNormal, flipped_texcoord).rgb) * 2.0);
	vec3 specularTexture = texture(samplerSpecular, flipped_texcoord).rgb;
	
	//Calculate normalTexture coords
	normalTexture = fs_in.TBN * normalTexture;

	vec4 outvec0 = vec4(0);
    vec4 outvec1 = vec4(0);

	//Position
	outvec0.xyz = fs_in.ws_coords;
	outvec0.w = 1.0;

	

	//Normal and diffuse packed in single texture
    outvec1.x = color2float(diffuseTexture);
    outvec1.y = color2float(normalTexture);
	outvec1.z = color2float(specularTexture);

    color0 = outvec0;
    color1 = outvec1;
}