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
	vec4 instancePos[3];
} ubo;


out VS_OUT
{
	mat3 TBN;
	vec3 ws_coords;
    vec2 texcoord;
} vs_out;


void main() 
{
	vec4 tmpPos = vec4(inPos,1.0) + ubo.instancePos[gl_InstanceIndex];

	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * tmpPos;
	
	//Calculate TBN
	mat3 mNormal = transpose(inverse(mat3(ubo.modelMatrix)));
	vec3 N = mNormal * normalize(inNormal);
    vec3 T = mNormal * normalize(inTangent);
    vec3 B = mNormal * normalize(inBinormal);
	vs_out.TBN = mat3(T, B, N);
	
	// Vertex position in world space
	vs_out.ws_coords =  vec3(ubo.modelMatrix * tmpPos);
	// GL to Vulkan coord space
	vs_out.ws_coords.y = -vs_out.ws_coords.y;
	vs_out.texcoord = inUv;
}
