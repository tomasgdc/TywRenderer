#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout (location = 0) in vec2 inUV;
layout (binding = 1) uniform sampler2D samplerColor;
layout (location = 0) out vec4 outFragColor;


 float n = 1.0f; // camera z near
 float f = 128.0f; // camera z far
float LinearizeDepth(float depth)
{
  float z = depth * 2.0f - 1.0f; //Back to NDC
  return (2.0f * n) / (f + n - z * (f - n));	
}


void main() 
{
	float depth = texture(samplerColor, inUV).r;
	outFragColor = vec4(vec3(depth), 1.0);
}