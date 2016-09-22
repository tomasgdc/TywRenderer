#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

in VS_OUT
{
	vec2 uv;
	uint textureType;
} fs_in;

layout (binding = 1) uniform sampler2D samplerColor;
layout (location = 0) out vec4 outFragColor;


void main() 
{
	vec3 color = vec3(1.0);

	//if world position texture
	if(fs_in.textureType == 0)
	{
		color  = texture(samplerColor, fs_in.uv).rgb;
	}
	else if(fs_in.textureType == 1) //if compressed diffuse and normal. Use diffuse texture only now
	{
		uvec4 data = texelFetch(samplerColor, fs_in.uv, 0);
		vec2 temp = unpackHalf2x16(data.y);

		//unpacked diffuse
		color  = vec3(unpackHalf2x16(data.x), temp.x);
	}
	else if(fs_in.textureType == 2) //if compressed diffuse and normal. Use normal texture only now
	{
		uvec4 data = texelFetch(samplerColor, fs_in.uv, 0);
		vec2 temp = unpackHalf2x16(data.y);

		//unpacked normal
		color  = normalize(vec3(temp.y, unpackHalf2x16(data.z)));
	}
	else if(fs_in.textureType == 3) //if specular texture
	{
		
		color  = texture(samplerColor, fs_in.uv).rgb;
	}

	outFragColor = vec4(color, 1.0);
}