#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	vec4 viewPos;
	float lodBias;
} ubo;

layout (location = 0) out vec2 outUv;
layout (location = 1) out float outLodBias;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;



out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	outUv = inUv;
	outLodBias = ubo.lodBias;

	vec3 worldPos = vec3(ubo.modelMatrix * vec4(inPos.xyz, 1.0));

	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);

	vec4 pos = ubo.modelMatrix * vec4(inPos, 1.0);
	outNormal = mat3(inverse(transpose(ubo.modelMatrix))) * inNormal;
	vec3 lightPos = vec3(0,0, 5);
	vec3 lPos = mat3(ubo.modelMatrix) * lightPos.xyz;
    outLightVec = lPos - pos.xyz;
    outViewVec = ubo.viewPos.xyz - pos.xyz;		
}
