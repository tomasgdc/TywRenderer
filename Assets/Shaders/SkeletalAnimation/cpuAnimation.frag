#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


in VS_OUT
{
	vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
    vec2 texcoord;
	float loadBias;
} fs_in;

layout (binding = 1) uniform sampler2D samplerDiffuse;

layout (location = 0) out vec4 outFragColor;
void main() 
{
	vec3 diffuseTexture = texture(samplerDiffuse,fs_in.texcoord).rgb;
	outFragColor = vec4(diffuseTexture.rgb, 1.0);
}