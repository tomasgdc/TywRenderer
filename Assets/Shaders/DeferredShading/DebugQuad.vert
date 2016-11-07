#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
	mat4 mvp;
	uint textureType;
} ubo;


layout(location = 3) out struct
{
	vec2 uv;
	//uint textureType;
} vs_out;

out gl_PerVertex 
{
    vec4 gl_Position;   
};
void main() 
{
	vs_out.uv = inUV;
	//vs_out.textureType = 3;
	gl_Position = ubo.mvp * vec4(inPos.xyz, 1.0);
}
