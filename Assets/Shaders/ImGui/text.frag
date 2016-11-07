#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const float smoothing = 1.0/16.0;
layout(location = 0, index = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;

layout (location = 3) in block
{
    vec4 Color;
    vec2 UV;
} In;


void main(void)
{
	fColor = In.Color * texture(sTexture, In.UV.st);
}