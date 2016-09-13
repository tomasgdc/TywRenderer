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
	float lodBias;
} ubo;


out VS_OUT
{
	vec3 ws_coords;
	vec3 normal;
    vec3 tangent;
    vec2 texcoord;
} vs_out;


void main() 
{
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
	
	
	// Vertex position in world space
	vs_out.ws_coords = vec3(ubo.model * tmpPos);
	// GL to Vulkan coord space
	vs_out.ws_coords.y = -outWorldPos.y;
	
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
    vs_out.tangent =  mNormal * normalize(inTangent);
	vs_out.normal =   mNormal * normalize(inNormal);
	vs_out.texcoord = inUv;
	
}
