#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;


layout (binding = 1) uniform sampler2D samplerColor;
layout (location = 0) out vec4 outFragColor;


void main() 
{
  vec4 color = texture(samplerColor, inUv);
  //outFragColor = vec4(color.rgb, 1.0);

  outFragColor = vec4(0.5, 0.1, 0.1, 1.0);
}