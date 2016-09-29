#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 depthMVP;
	vec4 instancePos[3];
} ubo;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
	vec4 tmpPos = vec4(inPos,1.0) + ubo.instancePos[gl_InstanceIndex];
	gl_Position =  ubo.depthMVP * tmpPos;
}