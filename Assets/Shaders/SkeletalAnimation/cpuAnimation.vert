#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location =  2) in vec4 jointWeight;
layout (location =  3) in ivec4 jointId;

#define MAX_BONES 110
layout (binding = 0) uniform UBO 
{
	mat4 BoneMatrix[MAX_BONES];
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 normal;
	vec4 viewPos;
	float lodBias;
} ubo;


out VS_OUT
{
	vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
    vec2 texcoord;
	float loadBias;
} vs_out;


void main() 
{

    mat4 boneTransform = ubo.BoneMatrix[jointId[0]] * jointWeight[0];
    boneTransform     += ubo.BoneMatrix[jointId[1]] * jointWeight[1];
    boneTransform     += ubo.BoneMatrix[jointId[2]] * jointWeight[2];
    boneTransform     += ubo.BoneMatrix[jointId[3]] * jointWeight[3];	
	

	vs_out.texcoord = inUv;
	vs_out.loadBias = ubo.lodBias;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * boneTransform * vec4(inPos.xyz, 1.0);
}
