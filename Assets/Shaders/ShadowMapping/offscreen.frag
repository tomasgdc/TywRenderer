#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 1) out vec4 fragmentdepth;

 float n = 1.0f; // camera z near
 float f = 128.0f; // camera z far
float LinearizeDepth(float depth)
{
  float z = depth * 2.0f - 1.0f; //Back to NDC
  return (2.0f * n) / (f + n - z * (f - n));	
}

void main() 
{	
	fragmentdepth = vec4(vec3(LinearizeDepth(gl_FragCoord.z)), 1.0);
}