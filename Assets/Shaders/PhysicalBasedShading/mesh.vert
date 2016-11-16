#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUv;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBinormal;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 normal;
	vec4 viewPos;
	vec3 lightPos;
} ubo;


layout (location = 5) out VS_OUT
{
    vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
	vec2 texcoord;
} vs_out;


void main() 
{
	//Light pos in model space
	vec3 lightPos = ubo.lightPos;

	//Calculate vertex position in view space
	mat4 mvMatrix = ubo.viewMatrix * ubo.modelMatrix;

	//Convert vertex pos to view space
	vec4 vertexPosition = mvMatrix *  vec4(inPos, 1.0);
	
	
	vs_out.lightDir = normalize(vec3(lightPos.xyz - vertexPosition.xyz));
	vs_out.eyeDir = normalize(-vertexPosition.xyz);
	vs_out.normal = normalize(mat3(ubo.normal) * inNormal); //problem
	vs_out.texcoord = inUv;
	
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
