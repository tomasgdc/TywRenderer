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



//Animation Includes
//#include <TywAnimation\MD5Anim\MD5Anim.h>


#define MD5VERSION 10
static uint32_t	 c_numVerts = 0;
static uint32_t	 c_numWeights = 0;
static uint32_t	 c_numWeightJoints = 0;



MD5Mesh::MD5Mesh():
	shader(nullptr), 
	numVerts(0),
	numTris(0),
	meshJoints(nullptr), 
	numMeshJoints(0), 
	maxJointVertDist(0.0f), 
	deformInfo(nullptr),
	surfaceNum(0)
{

}


MD5Mesh::~MD5Mesh() {
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
	
	//std::vector<vertIndex_t>	 vert;
	//std::vector<triangleIndex_t> tri;
	//std::vector<vertexWeight_t>  weight;
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
			tri.reserve(numTris*3);

			MD5_ParseTriangles(ptrFile, tri, numTris);
		}
		else if (strcmp(lineHeader, "numweights") == 0)
		{
			fscanf(ptrFile, "%i\n", &numWeights);
			weight.reserve(numVerts);

			MD5_ParseWeights(ptrFile, weight, numWeights);
			rval = 0; //stop reading.
		}
	}


//	deformInfo = TYW_NEW deformInfo_t;
//	deformInfo->verts = TYW_NEW drawVert[verts.size()];
//	deformInfo->indexes = TYW_NEW uint32_t[tri.size()];

//	deformInfo->numSourceVerts = verts.size();
//	deformInfo->numIndexes = tri.size();

	deformInfosVec.resize(verts.size());

	int b = 0;
	for (auto& i : verts)
	{

		glm::vec4 boneWeight(0);
		glm::ivec4 boneId(0);
		glm::vec3 v(0.0f, 0.0f, 0.0f);

		drawVert		 pose;
		for (int j = 0; j < i.numWeightsForVertex; j++)
		{
			//Engine::getInstance().Sys_Printf("tempWeight i = %i \n", i.firstWeightForVertex + j);
			vertexWeight_t& tempWeight = weight[i.firstWeightForVertex + j];
			
			//JointMat& joint = joints[tempWeight.jointId];
			JointQuat& joint = joints[tempWeight.jointId];

			// Convert the weight position from Joint local space to object space
			glm::vec3 rotPos = joint.q * tempWeight.pos;

			v += (joint.t + rotPos)  * tempWeight.jointWeight;

			if (j < 4)
			{
				boneWeight[j] = tempWeight.jointWeight;
				boneId[j] = static_cast<float>(i.firstWeightForVertex);
			}
		}
		deformInfosVec[b].vertex = v;
		deformInfosVec[b].tex = glm::vec2(i.texCoord.x, i.texCoord.y);
		deformInfosVec[b].boneWeight = boneWeight;
		deformInfosVec[b].boneId = boneId;


		//deformInfo->verts[b].Clear();
		//deformInfo->verts[b].vertex =  v;
		//deformInfo->verts[b].SetTexCoords(i.texCoord.x, i.texCoord.y);
		b++;
	}
	//std::copy(tri.begin(), tri.end(), deformInfo->indexes);
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



void RenderModelMD5::InitFromFile(std::string fileName, std::string filePath) {
	FILE* file = fopen((filePath + fileName).c_str(), "r");
	if (!file) {
		//Engine::getInstance().Sys_Printf("ERROR: Could not find %s", fileName.c_str());
		return;
	}

	//std::vector<JointMat> poseMat;
	std::vector<glm::mat4x4> poseMat;

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

			poseMat.reserve(numJoints);
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
					poseMat.push_back(boneMatrix);
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