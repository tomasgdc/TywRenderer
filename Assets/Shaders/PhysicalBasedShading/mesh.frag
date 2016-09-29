#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


in VS_OUT
{
    vec2 texcoord;
    vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
} fs_in;

layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerDiffuse;
layout (binding = 3) uniform sampler2D samplerSpecular;


layout (location = 0) out vec4 outFragColor;
void main() 
{
	vec3 diffuseTexture = vec3(1.0f, 0.5f, 0.0f);
	vec3 normalTexture  = vec3(1.0f);
	vec3 specularTexture = vec3(1.0f);
	
    
    vec3 V = normalize(fs_in.eyeDir);
    vec3 L = normalize(fs_in.lightDir);


    // Calculate R ready for use in Phong lighting.
    vec3 R = reflect(-L, fs_in.normal);


    // Calculate diffuse color with simple N dot L.
    vec3 diffuse = max(dot(fs_in.normal, V), 0.0) * diffuseTexture.rgb;
    // Uncomment this to turn off diffuse shading
    // diffuse = vec3(0.0);

    // Assume that specular albedo is white - it could also come from a texture
    vec3 specular_albedo = vec3(1.0);

    // Calculate Phong specular highlight
    vec3 specular =  max(pow(dot(R, V), 20.0), 0.0) * specularTexture;


    // Final color is diffuse + specular
    outFragColor = vec4(diffuse + specular, 1.0);
	//outFragColor = vec4(normalTexture.rgb, 1.0);
	//outFragColor = vec4(normalTexture.rgb, 1.0);
	//outFragColor = vec4(fs_in.normal.rgb, 1.0);
}