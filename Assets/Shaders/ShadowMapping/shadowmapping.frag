#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(location = 5) in struct
{
    vec2 texcoord;
    vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
	vec4 shadowCoord;
} fs_in;

layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerDiffuse;
layout (binding = 3) uniform sampler2D samplerSpecular;
layout (binding = 4) uniform sampler2D shadowMap;


layout (location = 0) out vec4 outFragColor;


#define ambient 0.1
float textureProj(vec4 P, vec2 off)
{
	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st + off ).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = ambient;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main() 
{
	//Flip coordinates
	vec2 flipped_texcoord = vec2(fs_in.texcoord.x, 1.0 - fs_in.texcoord.y);

	//Get textures
	vec3 diffuseTexture  =   texture(samplerDiffuse, flipped_texcoord).rgb;
	vec3 normalTexture  = normalize(vec3(1.0) - (texture(samplerNormal, flipped_texcoord).rgb) * 2.0);
	vec3 specularTexture =   texture(samplerSpecular, flipped_texcoord).rgb;


	//Get shadow
	float shadow = filterPCF(fs_in.shadowCoord / fs_in.shadowCoord.w);

	
    vec3 V = normalize(fs_in.eyeDir);
    vec3 L = normalize(fs_in.lightDir);


    // Calculate R ready for use in Phong lighting.
    vec3 R = reflect(-L, normalTexture);

    // Calculate diffuse color with simple N dot L.
	vec3 diffuse = max(dot(normalTexture, L), 0.0) * diffuseTexture;

    //vec3 diffuse = max(dot(fs_in.normal, V), 0.0) * diffuseTexture.rgb;
    // Uncomment this to turn off diffuse shading
    // diffuse = vec3(0.0);

    //Assume that specular albedo is white - it could also come from a texture
    //vec3 specular_albedo = vec3(1.0);

    //Calculate Phong specular highlight
    vec3 specular =  max(pow(dot(R, V), 20.0), 0.0) * specularTexture;


    // Final color is diffuse + specular
	outFragColor = vec4( (diffuse + specular)*shadow, 1.0);

	//Testing
	//outFragColor = vec4(normalTexture.rgb, 1.0);
	//outFragColor = vec4(normalTexture.rgb, 1.0);
	//outFragColor = vec4(fs_in.normal.rgb, 1.0);
	//outFragColor = vec4(diffuseTexture, 1.0);
	//outFragColor = vec4(normalTexture, 1.0);
	//outFragColor = vec4(specularTexture, 1.0);
}