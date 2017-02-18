#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUV;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};


layout (binding = 1) uniform   usampler2D  PosSpecularPacked;
layout (binding = 2) uniform   usampler2D  DiffuseNormalAndDepthPacked;
layout (binding = 3) uniform   sampler2D   ssaoImage; 

layout (binding = 4) uniform UBO 
{
	Light lights[17];
	vec4 viewPos;
} ubo;


layout (location = 0) out vec4 outFragColor;
#define lightCount 17
#define ambient 0.0
vec3 DefferedPass()
{
	ivec2 P0 = ivec2(inUV * textureSize(DiffuseNormalAndDepthPacked, 0));
	ivec2 P1 = ivec2(inUV * textureSize(PosSpecularPacked, 0));

	uvec3 uvec3_PosSpecularPacked = texelFetch(PosSpecularPacked, P1, 0).rgb;
	uvec4 uvec4_DiffuseNormalAndDepthPacked = texelFetch(DiffuseNormalAndDepthPacked, P0, 0);

	//Get position texture
	vec2 tempPosition0 = unpackHalf2x16(uvec3_PosSpecularPacked.x);
	vec2 tempPosSpec = unpackHalf2x16(uvec3_PosSpecularPacked.y);

	vec3 positionTexture = vec3(tempPosition0, tempPosSpec.x);
	// GL to Vulkan coord space
	positionTexture.y = -positionTexture.y;

	//Get Diffuse And Normal
	vec2 tempDiffuse0 = unpackHalf2x16(uvec4_DiffuseNormalAndDepthPacked.x);
	vec2 tempDiffAndNormal = unpackHalf2x16(uvec4_DiffuseNormalAndDepthPacked.y);
	vec2 tempNormal = unpackHalf2x16(uvec4_DiffuseNormalAndDepthPacked.z);

	vec3 diffuseTexture = vec3(tempDiffuse0, tempDiffAndNormal.x);
	vec3 normalTexture =  normalize(vec3(tempDiffAndNormal.y, tempNormal));

	//Get Specular
	vec2 tempSpecularYZ = unpackHalf2x16(uvec3_PosSpecularPacked.z);

	vec3 specularTexture = vec3(tempPosSpec.y, tempSpecularYZ);


	// Ambient part
	vec3 fragcolor  = diffuseTexture.rgb * ambient;
	for(int i = 0; i < lightCount; ++i)
	{
		// Vector to light
		vec3 L = ubo.lights[i].position.xyz - positionTexture;

		// Distance from light to fragment position
		float dist = length(L);

		// Viewer to fragment
		vec3 V = ubo.viewPos.xyz - positionTexture;
		V = normalize(V);
		
		if(dist < ubo.lights[i].radius)
		{
			// Light to fragment
			L = normalize(L);

			// Attenuation
			float atten = ubo.lights[i].radius / (pow(dist, 2.0) + 1.0);

			// Diffuse part
			float NdotL = max(0.0, dot(normalTexture, L));
			vec3 diff = ubo.lights[i].color * diffuseTexture.rgb * NdotL * atten;


			// calculate specular reflection only if
			// the surface is oriented to the light source
			float specularTerm = 0;
			if(dot(normalTexture, L) > 0)
			{
				//vec3 R = reflect(-L, N);
				vec3 H = normalize(L + V);
				specularTerm = pow(max(0.0, dot(normalTexture, H)), 16.0);
			}
			vec3 spec = ubo.lights[i].color * specularTexture * specularTerm * atten;

			//Final color
			fragcolor += diff + spec;	
		}	
	}  

	return fragcolor;
}


void main() 
 {
	float ssaoTexture = texture(ssaoImage, inUV).r;
	//outFragColor = vec4(ssaoTexture);

	outFragColor = vec4(DefferedPass(), 1.0);
}