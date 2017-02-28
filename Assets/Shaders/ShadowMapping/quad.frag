#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout (location = 0) in vec2 inUV;
layout (binding = 1) uniform sampler2D samplerColor;
layout (location = 0) out vec4 outFragColor;


const float NEAR_PLANE = 0.1f; //todo: specialization const
const float FAR_PLANE = 128.0f; //todo: specialization const 
float LinearizeDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}


void main() 
{
	float depth = texture(samplerColor, inUV).r;
	outFragColor = vec4(vec3(depth), 1.0);
}