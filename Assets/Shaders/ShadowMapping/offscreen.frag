#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//layout(location = 0) out vec4 color;
layout(location = 0) out float fragmentdepth;

float near = 1.0; 
float far  = 100.0; 
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main() 
{	
	fragmentdepth = LinearizeDepth(gl_FragCoord.z);
	//color = vec4(1.0, 1.0, 0.5, 1.0);
}