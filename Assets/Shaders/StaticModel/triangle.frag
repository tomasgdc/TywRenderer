#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(location = 5) in struct
{
    vec2 texcoord;
    vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
	float loadBias;
} fs_in;

layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerDiffuse;
layout (binding = 3) uniform sampler2D samplerSpecular;


layout (location = 0) out vec4 outFragColor;
void main() 
{
	vec2 flipped_texcoord = vec2(fs_in.texcoord.x, 1.0 - fs_in.texcoord.y);

	vec3 diffuseTexture = texture(samplerDiffuse, flipped_texcoord, fs_in.loadBias).rgb;
	vec3 normalTexture  = normalize(vec3(1.0) - texture(samplerNormal, flipped_texcoord).rgb * 2.0);
	vec3 specularTexture = texture(samplerSpecular, flipped_texcoord, fs_in.loadBias).rgb;
	
    
    vec3 V = normalize(fs_in.eyeDir);
    vec3 L = normalize(fs_in.lightDir);


    // Calculate R ready for use in Phong lighting.
    vec3 R = reflect(-L, normalTexture);

    // Calculate diffuse color with simple N dot L.
	vec3 diffuse = max(dot(normalTexture, L), 0.0) * diffuseTexture.rgb;

    // Assume that specular albedo is white - it could also come from a texture
    vec3 specular_albedo = vec3(1.0);

    // Calculate Phong specular highlight
    vec3 specular =  max(pow(dot(R, V), 20.0), 0.0) * specularTexture;


    // Final color is diffuse + specular
    outFragColor = vec4(diffuse + specular, 1.0);
}