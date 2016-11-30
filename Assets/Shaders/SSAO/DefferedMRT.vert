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


layout(location = 5) out struct
{
	mat3 TBN;
	vec3 ws_coords;
    vec2 texcoord;
} vs_out;


void main() 
{
	vec4 tmpPos = vec4(inPos,1.0) + ubo.instancePos[gl_InstanceIndex];
	mat4 mvMatrix  = ubo.viewMatrix * ubo.modelMatrix;
	
	
	//Calculate TBN
	mat3 mNormal = transpose(inverse(mat3(ubo.modelMatrix)));
	vec3 N = normalize(mNormal * inNormal);
	N.y = -N.y;
    vec3 T = normalize(mNormal * inTangent);
    vec3 B = cross(N, T);
	vs_out.TBN = mat3(T, B, N);
	
	// Vertex position in world space
	vs_out.ws_coords =  vec3(ubo.modelMatrix * tmpPos);


	// GL to Vulkan coord space
	vs_out.ws_coords.y = -vs_out.ws_coords.y;
	vs_out.texcoord = inUv;


	gl_Position = ubo.projectionMatrix *  mvMatrix * tmpPos;
}
