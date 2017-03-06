#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUV;
layout (binding = 1) uniform   sampler2D Image;

//Output 
layout (location = 0) out float outFragColor;

void main(void)
{
	const int blurRange = 2;
	int n = 0;
	vec2 texelSize = 1.0 / vec2(textureSize(Image, 0));
	float result = 0.0f;
	
	for(int x = -blurRange; x < blurRange; x++)
	{
		for(int y = -blurRange; y < blurRange; y++)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(Image, inUV + offset).r;
			n++;
		}
	}

	outFragColor = result / (float(n));
}