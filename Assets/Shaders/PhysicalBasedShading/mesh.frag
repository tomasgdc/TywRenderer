#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

/*
Sources
https://www.shadertoy.com/view/ld3SRr
http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf
https://chetanjags.wordpress.com/2015/08/26/image-based-lighting/
*/


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
	float lodBias;
	float ScreenGamma;
}lightData;

layout (binding = 2) uniform samplerCube samperCubeMap;
layout (location = 0) out vec4 outFragColor;


//http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf
float V_SmithGGXCorrelated ( float NdotL , float NdotV , float alphaG )
 {
	// Original formulation of G_SmithGGX Correlated
	// lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1)) * 0.5 f;
	// lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1)) * 0.5 f;
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NdotL * NdotV );

	// This is the optimize version
	 float alphaG2 = alphaG * alphaG ;
	// Caution : the " NdotL *" and " NdotV *" are explicitely inversed , this is not a mistake .
	float Lambda_GGXV = NdotL * sqrt (( - NdotV * alphaG2 + NdotV ) * NdotV + alphaG2 );
	float Lambda_GGXL = NdotV * sqrt (( - NdotL * alphaG2 + NdotL ) * NdotL + alphaG2 );

	return 0.5f / ( Lambda_GGXV + Lambda_GGXL );
}

//The Fresnel function describes the amount of light that reflects from a mirror surface given its index of refraction. 
float F_Schlick(float F0, float Ndot)
{
    return  F0 + (1.0f - F0) * pow(1.0f - Ndot, 5.0f);
}

//Self shadowing of microfacet
//http://graphicrants.blogspot.co.uk/2013/08/specular-brdf-reference.html
float G_CookTolerance(float NdotH, float NdotV, float VdotH, float NdotL)
{
    float NH2 = 2.0f * NdotH;
    float g1 = (NH2 * NdotV) / VdotH;
    float g2 = (NH2 * NdotL) / VdotH;
    float geoAtt = min(1.0f, min(g1, g2));
	return geoAtt;
}

//http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf
float D_GGX ( float NdotH , float m )
{
	 // Divide by PI is apply later
	 float m2 = m * m ;
	 float f = ( NdotH * m2 - NdotH ) * NdotH + 1.0F;
	 return m2 / (f * f) ;
 }
//http://www.frostbite.com/wp-content/uploads/2014/11/course_notes_moving_frostbite_to_pbr_v2.pdf
float Fr_DisneyDiffuse (float NdotV , float NdotL , float LdotH , float linearRoughness)
 {
	 float energyBias = mix(0.0f, 0.5f , linearRoughness );
	 float energyFactor = mix(1.0f, 1.0f / 1.51f , linearRoughness );
	 float fd90 = energyBias + 2.0f * LdotH * LdotH * linearRoughness ;

	 float lightScatter = F_Schlick (fd90 , NdotL );
	 float viewScatter = F_Schlick (fd90 , NdotV );

	return lightScatter * viewScatter * energyFactor ;
}

vec3 Color(vec3 normal, vec3 LightVec, vec3 EyeDirection)
{ 
    float NdotL = max(dot(normal, LightVec), 0.0);
    float specular = 0.0;

    //HalfVector -> Blin Phong reflection
    vec3 HalfVector = normalize((EyeDirection + LightVec));

    float NdotH = max(dot(normal, HalfVector), 0.0); 
    float NdotV = max(dot(normal, EyeDirection), 0.0); // note: this could also be NdotL, which is the same value
    float VdotH = max(dot(EyeDirection, HalfVector), 0.0);
	float LdotH = max(dot(LightVec, HalfVector), 0.0);


	 //Specular
	float alphaG = lightData.roughnessValue * lightData.roughnessValue;
    //float G = G_CookTolerance(NdotH, NdotV, VdotH, NdotL);
	float V = V_SmithGGXCorrelated (NdotL , NdotV , alphaG);
	float F = F_Schlick(lightData.F0, VdotH);
	float D = D_GGX(NdotH, alphaG);

    specular = (D  * V * F) / (4 * NdotV * NdotL);
    specular = (lightData.k + specular * (1.0 - lightData.k));

	//Diffuse
	float disneyDiffuse = Fr_DisneyDiffuse(NdotV , NdotL , LdotH, alphaG) / GLUS_PI;


	//Color
    return  (NdotL * lightData.Kd)  + specular ;
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