#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;


layout (binding = 0) uniform UBO 
{
	mat4 mvp;
	mat4 modelMatrix;
	mat4 shadowCoord;
} ubo;
layout (location = 0) out vec4 shadowCoordinates;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );


out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	gl_Position = ubo.mvp * vec4(inPos.xyz, 1.0);
	shadowCoordinates = ( biasMat * ubo.shadowCoord * ubo.modelMatrix) * vec4(inPos, 1.0);	
}
