#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUV;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout (binding = 4) uniform UBO 
{
	Light lights[6];
	vec4 viewPos;
} ubo;

layout (binding = 1) uniform   usampler2D  PosDepthAndSpecularPacked;
layout (binding = 2) uniform   usampler2D  DiffuseAndNormalPacked;
layout (binding = 3) uniform   sampler2D   ssaoImage; 


layout (location = 0) out vec4 outFragColor;

#define lightCount 6
#define ambient 0.0
vec3 DefferedPass()
{
	uvec4 uvec4_PosDepthAndSpecularPacked = texelFetch(PosDepthAndSpecularPacked, ivec2(gl_FragCoord.xy * 2.0), 0);
	uvec4 uvec4_DiffuseAndNormalPacked = texelFetch(DiffuseAndNormalPacked, ivec2(gl_FragCoord.xy * 2.0), 0);

	//Get position texture
	vec2 tempPosition0 = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.x);
	vec2 tempPosAndDepth = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.y);

	vec3 positionTexture = vec3(tempPosition0, tempPosAndDepth.x);

	//Get Diffuse And Normal
	vec2 tempDiffuse0 = unpackHalf2x16(uvec4_DiffuseAndNormalPacked.x);
	vec2 tempDiffAndNormal = unpackHalf2x16(uvec4_DiffuseAndNormalPacked.y);
	vec2 tempNormal = unpackHalf2x16(uvec4_DiffuseAndNormalPacked.z);

	vec3 diffuseTexture = vec3(tempDiffuse0, tempDiffAndNormal.x);
	vec3 normalTexture =  normalize(vec3(tempDiffAndNormal.y, tempNormal));

	//Get Specular
	vec2 tempSpecularXY = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.z);
	vec2 tempSpecularX = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.w);

	vec3 specularTexture = vec3(tempSpecularXY, tempSpecularX);


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
	outFragColor = vec4(DefferedPass(), 1.0);
}