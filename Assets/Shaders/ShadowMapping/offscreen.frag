#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 1) out vec4 fragmentdepth;

const float NEAR_PLANE = 0.1f; //todo: specialization const
const float FAR_PLANE = 128.0f; //todo: specialization const 
float LinearizeDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() 
{	
	fragmentdepth = vec4(vec3(LinearizeDepth(gl_FragCoord.z)), 1.0);
}