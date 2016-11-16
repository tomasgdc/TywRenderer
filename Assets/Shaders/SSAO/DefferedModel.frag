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

layout (binding = 1) uniform highp sampler2D samplerPosition;
layout (binding = 2) uniform highp sampler2D packedTexture;
layout (binding = 2) uniform highp sampler2D depthTexture;

// c_precision of 128 fits within 7 base-10 digits
const float c_precision = 128.0;
const float c_precisionp1 = c_precision + 1.0;

/*
\param value 3-component encoded float
\returns normalized RGB value
*/
vec3 float2color(float value) 
{
    vec3 color;
    color.r = mod(value, c_precisionp1) / c_precision;
    color.b = mod(floor(value / c_precisionp1), c_precisionp1) / c_precision;
    color.g = floor(value / (c_precisionp1 * c_precisionp1)) / c_precision;
    return color;
}
layout (location = 0) out vec4 outFragColor;


#define lightCount 6
#define ambient 0.0
vec3 DefferedPass(vec3 packedData)
{
	//Non packed texture
	vec3 positionTexture = texture(samplerPosition, inUV).rgb;

	//Unpacking data
	vec3 diffuseTexture = float2color(packedData.x);
	vec3 normalTexture =  normalize(float2color(packedData.y));
	vec3 specularTexture = float2color(packedData.z);

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
 	vec3 packedData = texture(packedTexture, inUV).rgb;
	outFragColor = vec4(DefferedPass(packedData), 1.0);
}