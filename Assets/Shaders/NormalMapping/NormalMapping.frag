#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inLightVec;
layout (location = 2) in vec3 inLightVecB;
layout (location = 3) in vec3 inSpecular;
layout (location = 4) in vec3 inEyeVec;
layout (location = 5) in vec3 inLightDir;
layout (location = 6) in vec3 inViewVec;
layout (location = 7) in float inLodBias;

layout (binding = 1) uniform sampler2D normalMap;
layout (binding = 2) uniform sampler2D diffuseMap;


layout (location = 0) out vec4 outFragColor;

void main() 
{
  vec3 specularColor = vec3(0.0, 0.0, 0.0);
  vec4 normalTex = 2.0 * texture(normalMap, inUV, inLodBias) -1.0;
  vec4 diffuseTex = texture(diffuseMap, inUV, inLodBias);

  float distSqr = dot(inLightVecB, inLightVecB);
  vec3 lVec = inLightVecB * inversesqrt(distSqr);

  vec3 N = normalize(normalTex.rgb);
  vec3 L = normalize(inViewVec);
  vec3 R = reflect(-L, N);

  float specular = pow(clamp(dot(R, lVec), 0.0, 1.0), 4.0);
  float diffuse = clamp(dot(lVec, N), 0.0, 1.0);
  

 outFragColor = vec4(diffuse*diffuseTex.rgb + 0.5 * specular * specularColor.rgb, 1.0);
    //outFragColor = vec4(diffuseTex.rgb + 0.5 * specular*specularColor.rgb, 1.0);
}