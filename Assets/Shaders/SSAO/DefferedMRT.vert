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
	vec3 posWorld;
	vec3 normal;
	vec2 depth;
    vec2 texcoord;
} vs_out;



void main() 
{
	vec4 tmpPos = vec4(inPos,1.0);
	mat4 mvMatrix  = ubo.viewMatrix * ubo.modelMatrix;
	mat4 mWorldViewProj = ubo.projectionMatrix *  mvMatrix;
	
	//Calculate TBN
	mat3 mNormal = transpose(inverse(mat3(ubo.modelMatrix)));
	vec3 N = normalize(mNormal * inNormal);
    vec3 T = normalize(mNormal * inTangent);
    vec3 B = normalize(mNormal * inBinormal);
	vs_out.TBN = mat3(T, B, N);
	
	// Vertex position in world space
	vs_out.posWorld =  vec3(ubo.modelMatrix * tmpPos);

	//texture coordinates
	vs_out.texcoord = inUv;

	// GL to Vulkan coord space
	//vs_out.ws_coords.y = -vs_out.ws_coords.y;
	
	//Face normal map
	mat3 normalMatrix = transpose(inverse(mat3(mvMatrix)));
	vs_out.normal = normalize(normalMatrix * inNormal);

	//Depth reconstruction
	vec4 worldViewPosition = mWorldViewProj * tmpPos;
	vs_out.depth = worldViewPosition.zw;

	//final vertex shader output
	gl_Position = worldViewPosition;
}
