#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;


layout (binding = 1) uniform sampler2D samplerColor;
layout (location = 0) out vec4 outFragColor;

const float smoothing = 1.0/16.0;
vec4  addColor = vec4(0.5, 0.5, 1, 1);
void main() 
{

  vec2 flipped_texcoord = vec2(inUv.x, 1.0 - inUv.y);
  //float distance = texture(samplerColor, flipped_texcoord).r;
  //float smoothWidth = fwidth(distance);
  //float alpha = smoothstep(0.5 - smoothWidth, 0.5 + smoothWidth, distance);
  //vec3 rgb = mix(vec3(alpha), addColor.rgb, alpha);	

  outFragColor = vec4(texture(samplerColor, flipped_texcoord).r);
  //outFragColor = addColor;
}