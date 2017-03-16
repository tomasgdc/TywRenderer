#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(location = 5) in struct
{
	mat3 TBN;
	vec3 posWorld;
	vec3 normal;
	vec2 depth;
    vec2 texcoord;
} fs_in;

layout (binding = 1) uniform sampler2D samplerDiffuse;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerSpecular;

//Position
layout (location = 0) out vec4 outPosition;

//Position, Specular
layout (location = 1) out vec4 outSpecular;

//Normal, Diffuse
layout (location = 2) out uvec4 outNormalDiffuse;

//Model normal, Depth
layout (location = 3) out vec4 outNormal;


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
 
const float NEAR_PLANE = 0.1f; //todo: specialization const
const float FAR_PLANE = 256.0f; //todo: specialization const 
float LinearizeDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}


void main() 
{	
	//Get correct coords
	vec2 flipped_texcoord = vec2(fs_in.texcoord.x, 1.0 - fs_in.texcoord.y);
	vec3 diffuseTexture = texture(samplerDiffuse, flipped_texcoord).rgb;
	vec3 normalTexture  = fs_in.TBN * normalize( (texture(samplerNormal, flipped_texcoord).xyz) * 2.0 - vec3(1.0));
	vec3 specularTexture = texture(samplerSpecular, flipped_texcoord).rgb;
	

    uvec4 outvec1 = uvec4(0);

	//Diffuse texture
	outvec1.x = packHalf2x16(diffuseTexture.xy);

	//Diffuse And Normal
	outvec1.y = packHalf2x16(vec2(diffuseTexture.z, normalTexture.x));

	//Normal
	outvec1.z = packHalf2x16(normalTexture.yz);


	//outPosition = vec4(fs_in.posWorld, fs_in.depth.x / gl_FragCoord.y);
	outPosition = vec4(fs_in.posWorld, gl_FragCoord.z);

    outSpecular = vec4(specularTexture, 1.0);
    outNormalDiffuse = outvec1;

	outNormal = vec4(fs_in.normal * 0.5 + 0.5, LinearizeDepth(gl_FragCoord.z));
}