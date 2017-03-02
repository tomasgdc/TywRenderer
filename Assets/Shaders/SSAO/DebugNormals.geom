#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(points) in;
layout(line_strip, max_vertices = 2) out;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	mat4 view;
	mat4 normal;
	vec4 viewPos;
	vec4 instancePos[3];
} ubo;

float length = 1.0;
vec3 color = vec3(1.0, 1.0, 1.0);

layout (location = 0) in vec3 inNormal[];
layout (location = 0) out vec3 outColor;

void main()
{
	mat4 mvp = ubo.projection * ubo.view * ubo.model;

	float normalLength = 1.0;
	for(int i=0; i<gl_in.length(); i++)
	{
		vec3 pos = gl_in[i].gl_Position.xyz;
		vec3 normal = inNormal[i].xyz;

		gl_Position = (mvp * vec4(pos, 1.0));
		outColor = vec3(1.0, 0.0, 0.0);
		EmitVertex();

		gl_Position = (mvp * vec4(pos + normal * normalLength, 1.0));
		outColor = vec3(0.0, 0.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}