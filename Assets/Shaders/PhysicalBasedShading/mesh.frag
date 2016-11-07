#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//CONSTANTS
#define GLUS_PI 3.1415926535897932384626433832795
//float roughnessValue = 0.1; // 0 : smooth, 1: rough
//float F0 = 0.8; // fresnel reflectance at normal incidence
//float k = 0.2; // fraction of diffuse reflection (specular reflection = 1 - k)

layout (location = 5) in VS_OUT
{
    vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
	vec2 texcoord;
} fs_in;


layout(binding = 1, std140) uniform LightData
{
	vec3 lightPos;
	vec3 Kd;
	float roughnessValue;				// 0 : smooth, 1: rough
	float F0;						// fresnel reflectance at normal incidence
	float k;				// fraction of diffuse reflection (specular reflection = 1 - k)
	float ScreenGamma;
}lightData;

layout (binding = 2) uniform samplerCube samperCubeMap;
layout (location = 0) out vec4 outFragColor;

float chiGGX(float v)
{
    return v > 0 ? 1 : 0;
}

float GGX_Distribution(float NoH, float alpha)
{
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float den = NoH2 * alpha2 + (1 - NoH2);
    return (chiGGX(NoH) * alpha2) / ( GLUS_PI * den * den );
}

vec3 Color(vec3 normal, vec3 LightVec, vec3 EyeDirection)
{ 
    //Get diffuse
    float NdotL = max(dot(normal, LightVec), 0.0);
    float specular = 0.0;

    //HalfVector -> Blin Phong reflection
    vec3 HalfVector = normalize((EyeDirection + LightVec));


    float NdotH = max(dot(normal, HalfVector), 0.0); 
    float NdotV = max(dot(normal, EyeDirection), 0.0); // note: this could also be NdotL, which is the same value
    float VdotH = max(dot(EyeDirection, HalfVector), 0.0);


    // geometric attenuation
    float NH2 = 2.0 * NdotH;
    float g1 = (NH2 * NdotV) / VdotH;
    float g2 = (NH2 * NdotL) / VdotH;
    float geoAtt = min(1.0, min(g1, g2));
    float mSquared = lightData.roughnessValue * lightData.roughnessValue;

    // roughness (or: microfacet distribution function)
    // beckmann distribution function -> Dark sides are too dark; Fix me? USE GGX
    float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
    float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
    float roughness = r1 * exp(r2);

    // fresnel
    // Schlick approximation
    float fresnel = pow(1.0 - VdotH, 5.0);
    fresnel *= (1.0 - lightData.F0);
    fresnel += lightData.F0;

    // IBL
    vec3 ibl_diffuse = texture(samperCubeMap,normal).xyz;
    vec3 ibl_reflection = texture(samperCubeMap,reflect(EyeDirection,normal)).xyz;
    vec3 refl = texture(samperCubeMap,reflect(EyeDirection,normal)).xyz;
    refl = mix(refl,ibl_reflection,(1.0-fresnel)*roughness);
    refl = mix(refl,ibl_reflection,roughness);

    //Specular
    specular = (fresnel  * geoAtt * GGX_Distribution(NdotH, lightData.roughnessValue)) / (NdotV * GLUS_PI);
    specular = (lightData.k + specular * (1.0 - lightData.k));
    
	//Color
    return  (NdotL * lightData.Kd) + specular;
}


void main() 
{
    vec3 colorLinear = Color(fs_in.normal, fs_in.lightDir, fs_in.eyeDir);

	//Gamma correction
	vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/lightData.ScreenGamma));
	//vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/2.2));

	//Output
    outFragColor = vec4(colorGammaCorrected, 1.0);
}