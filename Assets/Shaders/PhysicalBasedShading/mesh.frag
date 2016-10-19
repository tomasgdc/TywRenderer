#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define GLUS_PI 3.1415926535897932384626433832795

in VS_OUT
{
    vec2 texcoord;
    vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
	vec3 Kd;
	vec3 Ks;
} fs_in;

layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerDiffuse;
layout (binding = 3) uniform sampler2D samplerSpecular;


layout (location = 0) out vec4 outFragColor;




void main() 
{
    vec3 V = normalize(fs_in.eyeDir);
    vec3 L = normalize(fs_in.lightDir);


    // Calculate R ready for use in Phong lighting.
    vec3 R = reflect(-V, fs_in.normal);


    // Calculate diffuse color with simple N dot L.
    vec3 diffuse = max(dot(fs_in.normal, V), 0.0) * fs_in.Kd;
	
	//Specular light calculation
	vec3 specular =  max(pow(dot(R, V), 20.0), 0.0) * fs_in.Ks;


    // Final color is diffuse + specular
    outFragColor = vec4(diffuse + specular, 1.0);
}