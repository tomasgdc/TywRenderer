#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform highp  sampler2D  positionColor;
layout (binding = 2) uniform highp  sampler2D  packedTexture; //packed normal, diffuse, specular
layout (binding = 3) uniform highp  sampler2D  depthBuffer; 

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


void main() 
{
	vec3 packedData = texture(packedTexture, outUV.st).rgb;

	vec3 Components[5];
	Components[0] = texture(positionColor, outUV.st).rgb;

	//Unpacking data
	Components[1] = float2color(packedData.x);
	Components[2] =  normalize(float2color(packedData.y));
	Components[3] = float2color(packedData.z);
	Components[4] = vec3(1.0, 1.0, 1.0);

	highp int index = int(outUV.z);
	outFragColor.rgb = Components[index];
}