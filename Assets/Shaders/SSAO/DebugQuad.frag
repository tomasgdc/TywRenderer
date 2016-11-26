#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform   usampler2D  PosDepthAndSpecularPacked;
layout (binding = 2) uniform   usampler2D  DiffuseAndNormalPacked;
layout (binding = 3) uniform   sampler2D  ssaoImage; 

layout (location = 0) in vec3 outUV;
layout (location = 0) out vec4 outFragColor;


// c_precision of 128 fits within 7 base-10 digits
const float c_precision = 128.0;
const float c_precisionp1 = c_precision + 1.0;
/*
	param value 3-component encoded float
	returns normalized RGB value
*/
vec3 float2color(float value) 
{
    vec3 color;
    color.r = mod(value, c_precisionp1) / c_precision;
    color.b = mod(floor(value / c_precisionp1), c_precisionp1) / c_precision;
    color.g = floor(value / (c_precisionp1 * c_precisionp1)) / c_precision;
    return color;
}

/*
float LinearizeDepth(float depth)
{
  float n = 1.0; // camera z near
  float f = 128.0; // camera z far
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}
*/

vec3 VisFragment(int index)
{
	//Results
	vec3 result = vec3(1.0);

	if(index == 0)
	{
		uvec4 uvec4_PosDepthAndSpecularPacked = texelFetch(PosDepthAndSpecularPacked, ivec2(gl_FragCoord.xy * 5.0), 0);

		vec2 tempPosition0 = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.x);
		vec2 tempPosAndDepth = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.y);

		result = vec3(tempPosition0, tempPosAndDepth.x);
	}
	else if(index == 1)
	{
		uvec4 uvec4_DiffuseAndNormalPacked = texelFetch(DiffuseAndNormalPacked, ivec2(gl_FragCoord.xy * 5.0), 0);

		vec2 tempDiffuse0 = unpackHalf2x16(uvec4_DiffuseAndNormalPacked.x);
		vec2 tempDiffAndNormal = unpackHalf2x16(uvec4_DiffuseAndNormalPacked.y);

		result = vec3(tempDiffuse0.xy, tempDiffAndNormal.x);
	}
	else if(index == 2)
	{
		uvec4 uvec4_DiffuseAndNormalPacked = texelFetch(DiffuseAndNormalPacked, ivec2(gl_FragCoord.xy * 5.0), 0);

		vec2 tempDiffAndNormal = unpackHalf2x16(uvec4_DiffuseAndNormalPacked.y);
		vec2 tempNormal = unpackHalf2x16(uvec4_DiffuseAndNormalPacked.z);

		result = normalize(vec3(tempDiffAndNormal.y, tempNormal));
	}
	else if(index == 3)
	{
		uvec4 uvec4_PosDepthAndSpecularPacked = texelFetch(PosDepthAndSpecularPacked, ivec2(gl_FragCoord.xy * 5.0), 0);

		vec2 tempSpecularXY = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.z);
		vec2 tempSpecularX = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.w);

		result = vec3(tempSpecularXY, tempSpecularX.x);
	}
	else if(index == 4)
	{
		uvec4 uvec4_PosDepthAndSpecularPacked = texelFetch(PosDepthAndSpecularPacked, ivec2(gl_FragCoord.xy * 5.0), 0);

		vec2 tempPosAndDepth = unpackHalf2x16(uvec4_PosDepthAndSpecularPacked.y);
		result = vec3(tempPosAndDepth.y);
	}
	else if(index == 5)
	{
		float ssaoTexture = texture(ssaoImage,    outUV.st).r;
		result = vec3(ssaoTexture);
	}

	return result;
}


void main() 
{
	int index = int(outUV.z);
	outFragColor.rgb = VisFragment(index);
}