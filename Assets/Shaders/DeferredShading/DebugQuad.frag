#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

in VS_OUT
{
	vec2 uv;
	uint textureType;
} fs_in;

layout (binding = 1) uniform highp  sampler2D  positionColor;
layout (binding = 2) uniform highp  sampler2D  packedTexture; //packed normal, diffuse, specular

layout (location = 0) out vec4 outFragColor;


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


void main() 
{
	vec3 color = vec3(1.0);
	uint i = 0;

	//if world position texture
	if(i == 0)
	{
		color  = texture(positionColor, fs_in.uv).rgb;
	}
	else if(i == 1) //if compressed diffuse and normal. Use diffuse texture only now
	{
		vec3 data = texture(packedTexture, fs_in.uv).rgb;
		color = float2color(data.x);
	}
	else if(i == 2) //if compressed diffuse and normal. Use normal texture only now
	{
		vec3 data = texture(packedTexture, fs_in.uv).rgb;
		color = float2color(data.y);
	}
	else if(i == 3) //if specular texture
	{
	    vec3 data = texture(packedTexture, fs_in.uv).rgb;
		color = float2color(data.z);
	}

	outFragColor = vec4(color, 1.0);
}