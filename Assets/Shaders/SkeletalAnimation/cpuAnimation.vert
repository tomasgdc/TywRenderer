#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0)  in vec3 inPos;
layout (location = 1)  in vec3 inNormal;
layout (location = 2)  in vec3 inTangent;
layout (location = 3)  in vec3 inBinormal;
layout (location = 4)  in vec2 inUv;
layout (location = 5)  in vec4 jointWeight1;
layout (location = 6)  in vec4 jointWeight2;
layout (location = 7)  in ivec4 jointId1;
layout (location = 8)  in ivec4 jointId2;

#define MAX_BONES 110
layout (binding = 0) uniform UBO 
{
	mat4 BoneMatrix[MAX_BONES];
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 normal;
	vec4 viewPos;
} ubo;


layout(location = 9) out struct
{
    vec2 texcoord;
    vec3 eyeDir;
    vec3 lightDir;
    vec3 normal;
} vs_out;

void main() 
{
    mat4 boneTransform = ubo.BoneMatrix[jointId1[0]] * jointWeight1[0];
    boneTransform     += ubo.BoneMatrix[jointId1[1]] * jointWeight1[1];
    boneTransform     += ubo.BoneMatrix[jointId1[2]] * jointWeight1[2];
    boneTransform     += ubo.BoneMatrix[jointId1[3]] * jointWeight1[3];	

	boneTransform	  += ubo.BoneMatrix[jointId2[0]] * jointWeight2[0];
    boneTransform     += ubo.BoneMatrix[jointId2[1]] * jointWeight2[1];
    boneTransform     += ubo.BoneMatrix[jointId2[2]] * jointWeight2[2];
    boneTransform     += ubo.BoneMatrix[jointId2[3]] * jointWeight2[3];	


	//Light pos in model space
	vec3 lightPos = vec3(0,2, 10);

	//Calculate vertex position in view space
	mat4 mvMatrix = ubo.viewMatrix * ubo.modelMatrix;

	//Convert vertex pos to view space
	vec4 vertexPosition = mvMatrix *  vec4(inPos, 1.0);
	

	// Setup (t)angent-(b)inormal-(n)ormal matrix for converting
    // object coordinates into tangent space
	mat3 tbnMatrix;
	mat3 mNormal =  transpose(inverse(mat3(ubo.modelMatrix)));
    tbnMatrix[0] =  normalize(mNormal * inTangent);
	tbnMatrix[1] =  normalize(mNormal * inBinormal);
	tbnMatrix[2] =  normalize(mNormal * inNormal);

    
	// The light vector (L) is the vector from the point of interest to
    // the light. Calculate that and multiply it by the TBN matrix.
	vec3 LightVec = vec3(lightPos.xyz - vertexPosition.xyz);
	vs_out.lightDir = vec3(LightVec * tbnMatrix);
	//vs_out.lightDir = normalize(LightVec.xyz);


	// The view vector is the vector from the point of interest to the
    // viewer, which in view space is simply the negative of the position.
    // Calculate that and multiply it by the TBN matrix.
    //vs_out.eyeDir =   vec3(-vertexPosition * tbnMatrix);
    vs_out.eyeDir = vec3(-vertexPosition);

	
    // Pass the per-vertex normal so that the fragment shader can
    // turn the normal map on and off.
    vs_out.normal = tbnMatrix[2].xyz;


	vs_out.texcoord = inUv;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * boneTransform * vec4(inPos.xyz, 1.0);
}
