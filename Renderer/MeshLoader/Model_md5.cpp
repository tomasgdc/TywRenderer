/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>


//MeshLoader Includes
#include "Model_local.h"
#include "Material.h"
#include "MeshLoader\ImageManager.h"


//Vulkan Includes
#include "Vulkan\VulkanTextureLoader.h"



#define MD5VERSION 10
static uint32_t	 c_numVerts = 0;
static uint32_t	 c_numWeights = 0;
static uint32_t	 c_numWeightJoints = 0;


//Functions for normal, tangent, bitangent calculation
glm::vec3 CalculateNormalsMD5(const std::vector<uint32_t>& vertexIndices, const std::vector<MD5Mesh::meshStructure>& deformInfo, const int currentIndex);
glm::vec3 CalculateSmoothNormalsMD5(const std::vector<uint32_t>& vertexIndices, const std::vector<glm::vec3>& vertices, const int currentIndex);
void CalculateTangentAndBinormalMD5();//objMesh_t& pMesh, const int currentIndex);

MD5Mesh::MD5Mesh():
	shader(nullptr), 
	numVerts(0),
	numTris(0),
	meshJoints(nullptr), 
	numMeshJoints(0), 
	maxJointVertDist(0.0f), 
	deformInfo(nullptr),
	surfaceNum(0),
	numMaterials(0)
{

}


MD5Mesh::~MD5Mesh() 
{
	SAFE_DELETE_ARRAY(meshJoints);
}


void MD5_ParseVerts(FILE* ptrFile, std::vector<vertIndex_t> &vert, uint32_t	 numVerts) {
	vertIndex_t v;
	char strLine[256];
	char junk[256];
	for (int i = 0; i < numVerts; i++) {
		bool ferror = fgets(strLine, 256, ptrFile) == nullptr;
		if (!ferror) { 
			//puts(strLine); 
			int32_t rval = sscanf(strLine, "%s %i %s%f %f%s %i %i\n", junk, &v.index, junk, &v.texCoord.x, &v.texCoord.y, junk, &v.firstWeightForVertex, &v.numWeightsForVertex);
			if (rval != 8)break;
			vert.push_back(v);
		}
	}
}

void MD5_ParseTriangles(FILE* ptrFile, std::vector<uint32_t> &tri, uint32_t numsTriangles) {
	triangleIndex_t vIndex;
	char	strLine[256];
	char    junk[256];
	for (int i = 0; i < numsTriangles; i++) {
		bool ferror = fgets(strLine, 256, ptrFile) == nullptr;
		if (!ferror) {
		//	puts(strLine);
			int32_t rval = sscanf(strLine, "%s %s %i %i %i\n", junk, junk, &vIndex.indices[0], &vIndex.indices[1], &vIndex.indices[2]);
			if (rval != 5)break;

			tri.push_back(vIndex.indices[0]);
			tri.push_back(vIndex.indices[1]);
			tri.push_back(vIndex.indices[2]);
		}
	}
}


void MD5_ParseWeights(FILE* ptrFile, std::vector<vertexWeight_t> &weight, int32_t numWeights) {
	vertexWeight_t	w;
	char			strLine[256];
	char			junk[256];
	for (int i = 0; i < numWeights; i++) {
		bool ferror = fgets(strLine, 256, ptrFile) == nullptr;
		if (!ferror) {
		//	puts(strLine);
			int32_t rval = sscanf(strLine, "%s %s %i %f %s %f %f %f %s\n", junk, junk, &w.jointId, &w.jointWeight, junk, &w.pos.x, &w.pos.y, &w.pos.z, junk);
			if (rval != 9)break;
			weight.push_back(w);
		}
	}
}


void MD5_ParseMaterial(FILE* ptrFile, std::string& name)
{
	int32_t rval(1);
	while (rval != 0 && rval != EOF)
	{
		char lineHeader[128];
		rval = fscanf(ptrFile, "%s", lineHeader);

		if (strcmp(lineHeader, "diffusemap") == 0)
		{

		}
		else if (strcmp(lineHeader, "specularmap"))
		{

		}
	}
}

void MD5Mesh::ParseMesh(FILE* ptrFile, int numJoints, std::vector<JointQuat>& joints)//std::vector<JointMat>& joints) 
{
	char shaderName[128];
	uint32_t numVerts(0);
	uint32_t numWeights(0);
	int32_t rval(1);

	while(rval != 0 && rval != EOF)
	{
		char lineHeader[128];
		rval = fscanf(ptrFile, "%s", lineHeader);

		if (strcmp(lineHeader, "shader") == 0) 
		{
			fscanf(ptrFile, "%s\n", shaderName);
			//Engine::getInstance().Sys_Printf("%s \n", shaderName);
			
			shader = TYW_NEW Material;
			shader->setTexture(globalImage->GetImage(shaderName, "../../../Assets/Textures/", VkFormat::VK_FORMAT_UNDEFINED), false);
			numMaterials++;
		}
		else if (strcmp(lineHeader, "numverts") == 0) 
		{
			fscanf(ptrFile, "%i\n", &numVerts);
			verts.reserve(numVerts);

			MD5_ParseVerts(ptrFile, verts, numVerts);
		}
		else if (strcmp(lineHeader, "numtris") == 0) 
		{
			fscanf(ptrFile, "%i\n", &numTris);
			indexes.reserve(numTris*3);

			MD5_ParseTriangles(ptrFile, indexes, numTris);
		}
		else if (strcmp(lineHeader, "numweights") == 0)
		{
			fscanf(ptrFile, "%i\n", &numWeights);
			weight.reserve(numVerts);

			MD5_ParseWeights(ptrFile, weight, numWeights);
			rval = 0; //stop reading.
		}
	}


	//Instead if doing searching and sorting for vertex normals
	//I use first [] access as duplicates
	//Each time [vertexIndices[i]] is the same. I keep adding duplicate vertices position
	std::vector <std::vector<uint32_t>> duplicateVertices;
	duplicateVertices.resize(verts.size());
	deformInfosVec.resize(verts.size());

	int b = 0;
	for (auto& i : verts)
	{

		glm::mat3 boneWeight(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		glm::mat3 boneId(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 v(0.0f, 0.0f, 0.0f);
		drawVert		 pose;

		float* pBoneIndices = &boneId[0][0];
		float* pBoneWeights = &boneWeight[0][0];
		for (int j = 0; j < i.numWeightsForVertex; j++)
		{
			//MD5 have usually les than 9 bones per vertex. But usually in industry they use 4 bones per vertex
			//cant have more than 9 bones
			assert(j < 9);

			//Engine::getInstance().Sys_Printf("tempWeight i = %i \n", i.firstWeightForVertex + j);
			vertexWeight_t& tempWeight = weight[i.firstWeightForVertex + j];
			
			//JointMat& joint = joints[tempWeight.jointId];
			JointQuat& joint = joints[tempWeight.jointId];

			// Convert the weight position from Joint local space to object space
			glm::vec3 rotPos = joint.q * tempWeight.pos;

			v += (joint.t + rotPos)  * tempWeight.jointWeight;


			pBoneWeights[j] = tempWeight.jointWeight;
			pBoneIndices[j] = static_cast<float>(tempWeight.jointId);
		}
		deformInfosVec[b].vertex = v;
		deformInfosVec[b].tex = glm::vec2(i.texCoord.x, i.texCoord.y);
		
		deformInfosVec[b].boneWeight1 = glm::vec4(pBoneWeights[0], pBoneWeights[1], pBoneWeights[2], pBoneWeights[3]);
		deformInfosVec[b].boneWeight2 = glm::vec4(pBoneWeights[4], pBoneWeights[5], pBoneWeights[6], pBoneWeights[7]);
		deformInfosVec[b].boneId1     =	glm::ivec4(pBoneIndices[0], pBoneIndices[1], pBoneIndices[2], pBoneIndices[3]);
		deformInfosVec[b].boneId2     = glm::ivec4(pBoneIndices[4], pBoneIndices[5], pBoneIndices[6], pBoneIndices[7]);
		b++;
	}
	

	
	//Calculate Normasl
	for (uint32_t i = 0; i < indexes.size(); i += 3)
	{
		uint32_t& v0 = indexes[i];
		uint32_t& v1 = indexes[i+1];
		uint32_t& v2 = indexes[i+2];

		deformInfosVec[v0].normal = CalculateNormalsMD5(indexes, deformInfosVec, i);
		duplicateVertices[v0].push_back(v0);

		deformInfosVec[v1].normal = deformInfosVec[v0].normal;
		duplicateVertices[v1].push_back(v1);

		deformInfosVec[v2].normal = deformInfosVec[v0].normal;
		duplicateVertices[v2].push_back(v2);
	}
	

	
	//Create new smooth normals
	glm::vec3 newNormal;
	for (uint32_t i = 0; i < indexes.size(); i++)
	{
		glm::vec3 newNormal(0, 0, 0);
		const std::vector<uint32_t>& duplicates = duplicateVertices[indexes[i]];
		for (auto& duplicatePos : duplicates)
		{
			newNormal += deformInfosVec[duplicatePos].normal;
		}
		deformInfosVec[indexes[i]].normal = glm::normalize(newNormal);
	}




	//Calculate Tangent and Bittangent
	for (int i = 0; i < indexes.size(); i += 3)
	{
		uint32_t& index1 = indexes[i];
		uint32_t& index2 = indexes[i+1];
		uint32_t& index3 = indexes[i+2];


		const glm::vec3& v1 = deformInfosVec[index1].vertex;
		const glm::vec3& v2 = deformInfosVec[index2].vertex;
		const glm::vec3& v3 = deformInfosVec[index3].vertex;


		const glm::vec2& uv1 = deformInfosVec[index1].tex;
		const glm::vec2& uv2 = deformInfosVec[index2].tex;
		const glm::vec2& uv3 = deformInfosVec[index3].tex;

		//calculate edges
		glm::vec3 Edge1 = v2 - v1;
		glm::vec3 Edge2 = v3 - v1;

		//calculate uv edges
		glm::vec2 Edge1Uv = uv2 - uv1;
		glm::vec2 Edge2Uv = uv3 - uv1;


		float r = 1.0f / (Edge1Uv.x * Edge2Uv.y - Edge1Uv.y * Edge2Uv.x);
		glm::vec3 tangent = (Edge1 * Edge2Uv.y - Edge2 * Edge1Uv.y)*r;
		glm::vec3 bitangent = (Edge2 * Edge1Uv.x - Edge1 * Edge2Uv.x)*r;

		deformInfosVec[index1].tangent = tangent;
		deformInfosVec[index2].tangent = tangent;
		deformInfosVec[index3].tangent = tangent;

		deformInfosVec[index1].binormal = bitangent;
		deformInfosVec[index2].binormal = bitangent;
		deformInfosVec[index3].binormal = bitangent;
	}

	
	for (auto meshData: deformInfosVec)
	{
		const glm::vec3 & n = meshData.normal;
		const glm::vec3 & t = meshData.tangent;
		meshData.tangent = glm::orthonormalize(t, n);

		meshData.binormal = glm::cross(meshData.tangent, n);
	}
}

bool RenderModelMD5::ParseJoint(FILE* ptrFile, MD5Joint& joint, JointQuat& pose) {
	char			strLine[256];
	char			junk[256];

		bool ferror = fgets(strLine, 256, ptrFile) == nullptr;
		if (!ferror) 
		{
			//breaks	
//			puts(strLine);
			int32_t rval = sscanf(strLine, "%s %i %s %f %f %f %s %s %f %f %f %s", joint.name, &joint.parentID, junk, &pose.t.x, &pose.t.y, &pose.t.z, junk, junk, &pose.q.x, &pose.q.y, &pose.q.z, junk);
			if (rval != 12)
			{
				return false;
			}
			JointQuat::CalculateQuatW(pose.q);
		}
	return true;
}

glm::vec3 CalculateNormalsMD5(const std::vector<uint32_t>& vertexIndices, const std::vector<MD5Mesh::meshStructure>& deformInfo, const int currentIndex)
{
	glm::vec3 Edge1 = deformInfo[vertexIndices[(currentIndex + 1)]].vertex - deformInfo[vertexIndices[currentIndex]].vertex;
	glm::vec3 Edge2 = deformInfo[vertexIndices[(currentIndex + 2)]].vertex - deformInfo[vertexIndices[currentIndex]].vertex;

	return glm::normalize(glm::cross(Edge1, Edge2));
}


void CalculateTangentAndBinormalMD5()//objMesh_t& pMesh, const int currentIndex)
{
	/*
	//calculate edges
	glm::vec3 Edge1 = pMesh.vertices[currentIndex + 1] - pMesh.vertices[currentIndex];
	glm::vec3 Edge2 = pMesh.vertices[currentIndex + 2] - pMesh.vertices[currentIndex];


	//calculate uv edges
	glm::vec2 Edge1Uv = pMesh.uvs[currentIndex + 1] - pMesh.uvs[currentIndex];
	glm::vec2 Edge2Uv = pMesh.uvs[currentIndex + 2] - pMesh.uvs[currentIndex];


	pMesh.tangent[currentIndex] = glm::vec3(0, 0, 0);
	pMesh.binormal[currentIndex] = glm::vec3(0, 0, 0);

	float r = 1.0f / (Edge1Uv.x * Edge2Uv.y - Edge1Uv.y * Edge2Uv.x);
	glm::vec3 tangent = (Edge1 * Edge2Uv.y - Edge2 * Edge1Uv.y)*r;
	glm::vec3 bitangent = (Edge2 * Edge1Uv.x - Edge1 * Edge2Uv.x)*r;

	//t = tangent;
	//b = bitangent;
	TangentAndBinormalCalculator(Edge1, Edge2, Edge1Uv, Edge2Uv, pMesh.normals[currentIndex], pMesh.tangent[currentIndex], pMesh.binormal[currentIndex]);
	*/
}


glm::vec3 CalculateSmoothNormalsMD5(const std::vector<uint32_t>& vertexIndices, const std::vector<glm::vec3>& vertices, const int currentIndex)
{
	//calculate smooth normals
	glm::vec3 normal(0, 0, 0);
	for (int j = currentIndex; j < vertexIndices.size(); j += 3)
	{
		if (vertexIndices[j] == vertexIndices[currentIndex] && (j + 2) < vertexIndices.size())
		{
			glm::vec3 Edge1 = vertices[vertexIndices[j + 1]] - vertices[vertexIndices[j]];
			glm::vec3 Edge2 = vertices[vertexIndices[j + 2]] - vertices[vertexIndices[j]];

			normal += glm::normalize(glm::cross(Edge1, Edge2));
		}
	}
	return glm::normalize(normal);
}

void RenderModelMD5::InitFromFile(std::string fileName, std::string filePath) {
	FILE* file = fopen((filePath + fileName).c_str(), "r");
	if (!file) {
		//Engine::getInstance().Sys_Printf("ERROR: Could not find %s", fileName.c_str());
		return;
	}

	//std::vector<JointMat> poseMat;
	//std::vector<glm::mat4x4> poseMat;

	uint32_t version	= 0;
	uint32_t numJoints	= 0;
	uint32_t numMeshes	= 0;
	int32_t  rval		= 1;
	while (rval != EOF && rval != 0) {
		char lineHeader[128];
		rval = fscanf(file, "%s", lineHeader);

		if (strcmp(lineHeader, "MD5Version") == 0) {
			fscanf(file, "%i\n", &version);
			if (version != MD5VERSION) {
				//Engine::getInstance().Sys_Printf("MD5Version is %i not supported, expected version %i\r\n", version, MD5VERSION);
				break;
			}
		}
		else if (strcmp(lineHeader, "numJoints") == 0) {
			fscanf(file, "%i\n", &numJoints);
			
		}
		else if(strcmp(lineHeader, "numMeshes") == 0) {
			fscanf(file, "%i\n", &numMeshes);
			if (numMeshes < 0) {
				//Engine::getInstance().Sys_Printf("Invalid size %i \n\r", numMeshes);
				break;
			}
		}
		else if(strcmp(lineHeader, "joints") == 0) {
			char junk[256];
			fgets(junk, 256, file); //skip {

			//poseMat.reserve(numJoints);
			joints.reserve(numJoints);
			defaultPose.reserve(numJoints);
			inverseBindPose.reserve(numJoints);
			for (int i = 0; i < numJoints; i++)
			{
				MD5Joint joint;
				JointQuat pose;
				//JointMat  pMat;
				if (ParseJoint(file, joint, pose))
				{
					joints.push_back(joint);
					defaultPose.push_back(pose);


					glm::mat4x4 boneTranslation = glm::translate(pose.t);
					glm::mat4x4 boneRotation = glm::toMat4(pose.q);

					glm::mat4x4 boneMatrix = boneTranslation * boneRotation;
					glm::mat4x4 inverseBoneMatrix = glm::inverse(boneMatrix);

					//pMat.SetRotation(bonem);
					//pMat.SetTranslation(pose.t);
					//poseMat.push_back(pMat);
					//poseMat.push_back(boneMatrix);
					inverseBindPose.push_back(inverseBoneMatrix);
				}
			}
			
		}
		else if(strcmp(lineHeader, "mesh") == 0) 
		{

			for (int i = 0; i < numMeshes; i++)
			{
				MD5Mesh mesh;
				meshes.push_back(mesh);
				meshes[i].ParseMesh(file, numJoints, defaultPose);
			}
		}
	}
	fclose(file);
}

void MD5Mesh::UpdateMesh(const MD5Mesh *mesh,  std::vector<JointQuat>& joints, const JointMat *entJointsInverted, deformInfo_t *surf)
{
	int b = 0;
	for (auto& i : verts)
	{

		glm::vec3 v(0.0f, 0.0f, 0.0f);
		for (int j = 0; j < i.numWeightsForVertex; j++)
		{
			//Engine::getInstance().Sys_Printf("tempWeight i = %i \n", i.firstWeightForVertex + j);
			vertexWeight_t& tempWeight = weight[i.firstWeightForVertex + j];
			JointQuat& joint = joints[tempWeight.jointId];

			JointMat mat;


			/*Convert quat to mat4*/
			glm::mat4x4 quatMat4;
			float	wx, wy, wz;
			float	xx, yy, yz;
			float	xy, xz, zz;
			float	x2, y2, z2;

			x2 = joint.q.x + joint.q.x;
			y2 = joint.q.y + joint.q.y;
			z2 = joint.q.z + joint.q.z;

			xx = joint.q.x * x2;
			xy = joint.q.x * y2;
			xz = joint.q.x * z2;

			yy = joint.q.y * y2;
			yz = joint.q.y * z2;
			zz = joint.q.z * z2;

			wx = joint.q.w * x2;
			wy = joint.q.w * y2;
			wz = joint.q.w * z2;

			quatMat4[0][0] = 1.0f - (yy + zz);
			quatMat4[0][1] = xy - wz;
			quatMat4[0][2] = xz + wy;
			quatMat4[0][3] = 0.0f;

			quatMat4[1][0] = xy + wz;
			quatMat4[1][1] = 1.0f - (xx + zz);
			quatMat4[1][2] = yz - wx;
			quatMat4[1][3] = 0.0f;

			quatMat4[2][0] = xz - wy;
			quatMat4[2][1] = yz + wx;
			quatMat4[2][2] = 1.0f - (xx + yy);
			quatMat4[2][3] = 0.0f;

			quatMat4[3][0] = 0.0f;
			quatMat4[3][1] = 0.0f;
			quatMat4[3][2] = 0.0f;
			quatMat4[3][3] = 1.0f;
			/*Convert end*/


			mat.SetRotation(quatMat4);
			mat.SetTranslation(joint.t);

			// Convert the weight position from Joint local space to object space
			glm::vec3 rotPos = mat * tempWeight.pos;

			v += (mat.GetTranslation() + rotPos)  * tempWeight.jointWeight;
		}
		deformInfo->verts[b].Clear();
		deformInfo->verts[b].vertex = v;
		deformInfo->verts[b].SetTexCoords(i.texCoord.x, i.texCoord.y);
		b++;
	}

	/*
	for (int i = 0; i < tri.size(); i++)
	{

		for (int j = 0; j < 3; j++)
		{
			uint32_t index = tri[i].indices[j];
			surf->verts[i * 3 + j].Clear();
			surf->verts[i * 3 + j].tex = basePose[index].tex;
			surf->verts[i * 3 + j].vertex = basePose[index].vertex;
		}
	}
	*/
}


void RenderModelMD5::Clear(VkDevice device)
{
	//Delete texture data
	for (auto& mesh : meshes)
	{
		for (int i = 0; i < mesh.numMaterials; i++)
		{
			mesh.shader[i].Clear(device);
		}
		//SAFE_DELETE_ARRAY(mesh.shader);
	}
}