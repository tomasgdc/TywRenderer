/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>

//Renderer Includes
#include "Model_obj.h"

//Geometry Includes
#include "Geometry\TangentAndBinormalCalculator.hpp"

typedef struct {
	objModel_t	*model;
	objObject_t *currentObject;
}obj_t;
static obj_t objGlobal;

bool hasVer, hasNormal, hasTexCoord;

//Functions for normal, tangent, bitangent calculation
glm::vec3 CalculateNormals(const std::vector< unsigned int >& vertexIndices, const std::vector<glm::vec3>& temp_vertices, const int currentIndex);
glm::vec3 CalculateSmoothNormals(const std::vector< unsigned int >& vertexIndices, const std::vector<glm::vec3>& temp_vertices, const int currentIndex);
void CalculateTangentAndBinormal(objMesh_t& pMesh, const int currentIndex);


/*
===================
OBJ_ParseVertex
===================
*/
void OBJ_ParseVertex(FILE * file, std::vector<glm::vec3>& v) {
	glm::vec3 vertex;
	fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
	v.push_back(vertex);
}

/*
===================
OBJ_ParseTexCoord
===================
*/
void OBJ_ParseTextCoord(FILE * file, std::vector<glm::vec2>& v) {
	glm::vec2 uv;
	uint32_t dump; //w coord which is not needed
	fscanf(file, "%f %f %f\n", &uv.x, &uv.y, &dump);
	v.push_back(uv);
}

/*
===================
OBJ_ParseNormal
===================
*/
void OBJ_ParseNormal(FILE * file, std::vector<glm::vec3>& v) {
	glm::vec3 normal;
	fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
	v.push_back(normal);
}

/*
=========================
OBJ_ParseVerntexIndices
=========================
*/
void OBJ_ParseVertexIndices(FILE* file, std::vector< unsigned int >&  vertexIndices) {
	unsigned int vertexIndex[3];
	fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
	vertexIndices.push_back(vertexIndex[0] - 1);
	vertexIndices.push_back(vertexIndex[1] - 1);
	vertexIndices.push_back(vertexIndex[2] - 1);
}

/*
=========================
OBJ_ParseVertexNormalIndices
=========================
*/
void OBJ_ParseVertexNormalIndices(FILE* file,
	std::vector< unsigned int >&  vertexIndices,
	std::vector< unsigned int >&  normalIndices) {
	unsigned int vertexIndex[3], normalIndex[3];
	fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
	vertexIndices.push_back(vertexIndex[0] - 1);
	vertexIndices.push_back(vertexIndex[1] - 1);
	vertexIndices.push_back(vertexIndex[2] - 1);
	normalIndices.push_back(normalIndex[0] - 1);
	normalIndices.push_back(normalIndex[1] - 1);
	normalIndices.push_back(normalIndex[2] - 1);
}

/*
==============================
OBJ_ParseVertexTextureIndices
==============================
*/
void OBJ_ParseVertexTextureIndices(FILE* file,
	std::vector< unsigned int >&  vertexIndices,
	std::vector< unsigned int >&  uvIndices) {
	unsigned int vertexIndex[3], uvIndex[3];
	fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
	vertexIndices.push_back(vertexIndex[0] - 1);
	vertexIndices.push_back(vertexIndex[1] - 1);
	vertexIndices.push_back(vertexIndex[2] - 1);
	uvIndices.push_back(uvIndex[0] - 1);
	uvIndices.push_back(uvIndex[1] - 1);
	uvIndices.push_back(uvIndex[2] - 1);
}

/*
=====================================
OBJ_ParseVertexTextureNormalIndices
=====================================
*/
void OBJ_ParseVertexTextureNormalIndices(FILE* file,
	std::vector< unsigned int >&  vertexIndices,
	std::vector< unsigned int >&  uvIndices,
	std::vector< unsigned int >&  normalIndices) {
	unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
	fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
	vertexIndices.push_back(vertexIndex[0] - 1);
	vertexIndices.push_back(vertexIndex[1] - 1);
	vertexIndices.push_back(vertexIndex[2] - 1);
	uvIndices.push_back(uvIndex[0] - 1);
	uvIndices.push_back(uvIndex[1] - 1);
	uvIndices.push_back(uvIndex[2] - 1);
	normalIndices.push_back(normalIndex[0] - 1);
	normalIndices.push_back(normalIndex[1] - 1);
	normalIndices.push_back(normalIndex[2] - 1);
}

/*
===================
OBJ_ParseFace
===================
*/
bool OBJ_ParseFace(FILE* file,
	std::vector< unsigned int >&  vertexIndices,
	std::vector< unsigned int >&  uvIndices,
	std::vector< unsigned int >&  normalIndices) {

	if (hasVer && !hasNormal && !hasTexCoord) {
		OBJ_ParseVertexIndices(file, vertexIndices);
	}
	else if (hasVer && hasNormal && !hasTexCoord) {
		OBJ_ParseVertexNormalIndices(file, vertexIndices, normalIndices);
	}
	else if (hasVer && !hasNormal && hasTexCoord) {
		OBJ_ParseVertexTextureIndices(file, vertexIndices, uvIndices);
	}
	else if (hasVer && hasNormal && hasTexCoord) {
		OBJ_ParseVertexTextureNormalIndices(file, vertexIndices, uvIndices, normalIndices);
	}
	else {
		printf("ERROR: OBJ_ParseFace: wrong format \r\n");
		return false;
	}
	return true;
}



/*
========================
OBJ_ParseObject
========================
*/
int OBJ_ParseObject(FILE* file, const char* objectName,
	std::vector<glm::vec3>& temp_vertices,
	std::vector<glm::vec2>& temp_uvs,
	std::vector<glm::vec3>& temp_normals) {

	char materialName[128];
	memset(materialName, 0, sizeof(materialName));

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	int numVertices = 0, numNormals = 0, numTexCoord = 0, numFaces = 0;
	hasVer = false, hasNormal = false, hasTexCoord = false;
	int res = 0;
	while (1) {
		char lineHeader[128];
		res = fscanf(file, "%s", lineHeader);
		if (res == EOF) { break; }
		else if (strcmp(lineHeader, "o") == 0 || strcmp(lineHeader, "g") == 0) {
			char objectName[128];
			fscanf(file, "%s\n", objectName);
			res = OBJ_ParseObject(file, objectName, temp_vertices, temp_uvs, temp_normals);
		}

		if (strcmp(lineHeader, "v") == 0) {
			OBJ_ParseVertex(file, temp_vertices);
			numVertices++;
			hasVer = true;
		}
		else if ((strcmp(lineHeader, "vt") == 0)) {
			OBJ_ParseTextCoord(file, temp_uvs);
			numTexCoord++;
			hasTexCoord = true;
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			OBJ_ParseNormal(file, temp_normals);
			numNormals++;
			hasNormal = true;
		}
		else if (strcmp(lineHeader, "usemtl") == 0) {
			fscanf(file, "%s\n", materialName);
		}
		else if ((strcmp(lineHeader, "f") == 0)) {
			if (!OBJ_ParseFace(file, vertexIndices, uvIndices, normalIndices)) {
				return 0;
			}
			numFaces++;
		}
	}
	//Create new object
	objObject_t *object = TYW_NEW objObject_t;
	memset(object, 0, sizeof(objObject_t));

	//get object group name and material group name
	strcpy(object->name, objectName);
	strcpy(object->mat_name, materialName);

	objGlobal.model->objects.push_back(object);
	objGlobal.currentObject = object;

	//check against nullptr
	objMesh_t* pMesh = &objGlobal.currentObject->mesh;

	//Create needed size
	pMesh->vertices = TYW_NEW glm::vec3[numFaces * 3];
	pMesh->normals = TYW_NEW glm::vec3[numFaces * 3];
	pMesh->tangent = TYW_NEW glm::vec3[numFaces * 3];
	pMesh->binormal = TYW_NEW glm::vec3[numFaces * 3];
	pMesh->uvs = TYW_NEW glm::vec2[numFaces * 3];
	pMesh->faces = nullptr;

	pMesh->numVertexes = numFaces * 3;
	pMesh->numUvs = numFaces * 3;
	pMesh->numNormals = numFaces * 3;
	pMesh->numTangets = numFaces * 3;
	pMesh->numBinormals = numFaces * 3;
	pMesh->numFaces = numFaces;




	//DEBUG
	printf("Mesh:     %s \r\n", objectName);
	printf("VERTEX:	  %i \r\n", numVertices);
	printf("TEXCOORD: %i \r\n", numTexCoord);
	printf("NORMALS:  %i \r\n", numNormals);
	printf("FACES:    %i \r\n", numFaces);
	printf("\r\n");

	

	//Instead if doing searching and sorting for vertex normals
	//I use first [] access as duplicates
	//Each time [vertexIndices[i]] is the same. I keep adding duplicate vertices position
	std::vector <std::vector<uint32_t>> duplicateVertices;
	duplicateVertices.resize(temp_vertices.size());

	for (uint32_t i = 0; i < pMesh->numFaces * 3; i+=3) 
	{
		//First Vertex
		pMesh->vertices[i] = temp_vertices[vertexIndices[i]];
		pMesh->uvs[i] = temp_uvs[uvIndices[i]];
		pMesh->normals[i] = pMesh->normals[i] = CalculateNormals(vertexIndices, temp_vertices, i);
		pMesh->tangent[i] = glm::vec3(0, 0, 0);
		pMesh->binormal[i] = glm::vec3(0, 0, 0);
		duplicateVertices[vertexIndices[i]].push_back(i);

		//Second Vertex
		pMesh->vertices[i+1] = temp_vertices[vertexIndices[i+1]];
		pMesh->uvs[i+1] = temp_uvs[uvIndices[i+1]];
		pMesh->normals[i+1] = pMesh->normals[i];
		pMesh->tangent[i+1] = glm::vec3(0, 0, 0);
		pMesh->binormal[i+1] = glm::vec3(0, 0, 0);
		duplicateVertices[vertexIndices[i+1]].push_back(i+1);

		//Third Vertex
		pMesh->vertices[i+2] = temp_vertices[vertexIndices[i+2]];
		pMesh->uvs[i+2] = temp_uvs[uvIndices[i+2]];
		pMesh->normals[i+2] = pMesh->normals[i];
		pMesh->tangent[i+2] = glm::vec3(0, 0, 0);
		pMesh->binormal[i+2] = glm::vec3(0, 0, 0);
		duplicateVertices[vertexIndices[i+2]].push_back(i+2);
	}

	//Create new smooth normals
	glm::vec3* newNormals = TYW_NEW glm::vec3[pMesh->numVertexes];
	for (uint32_t i = 0; i < pMesh->numVertexes; i++)
	{
		newNormals[i] = glm::vec3(0, 0, 0);
		const std::vector<uint32_t>& duplicates = duplicateVertices[vertexIndices[i]];
		for (auto& duplicatePos: duplicates)
		{
			newNormals[i] += pMesh->normals[duplicatePos];
		}
		newNormals[i] = glm::normalize(newNormals[i]);
	}

	//Delete old normals and assign new ones
	SAFE_DELETE_ARRAY(pMesh->normals)
	pMesh->normals = newNormals;


	//Calculate Tangent and Bittangent
	for (int i = 0; i < pMesh->numVertexes; i+=3)
	{
		glm::vec3& v0 = pMesh->vertices[i];
		glm::vec3& v1 = pMesh->vertices[i + 1];
		glm::vec3& v2 = pMesh->vertices[i + 2];


		glm::vec2& uv0 = pMesh->uvs[i];
		glm::vec2& uv1 = pMesh->uvs[i+1];
		glm::vec2& uv2 = pMesh->uvs[i+2];

		glm::vec3 Edge1 = v1 - v0;
		glm::vec3 Edge2 = v2 - v0;

		float DeltaU1 = uv1.x - uv0.x;
		float DeltaV1 = uv1.y - uv0.y;
		float DeltaU2 = uv2.x - uv0.x;
		float DeltaV2 = uv2.y - uv0.y;

		float f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		glm::vec3 Tangent, Bitangent;

		Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
		Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
		Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

		Bitangent.x = f * (-DeltaU2 * Edge1.x - DeltaU1 * Edge2.x);
		Bitangent.y = f * (-DeltaU2 * Edge1.y - DeltaU1 * Edge2.y);
		Bitangent.z = f * (-DeltaU2 * Edge1.z - DeltaU1 * Edge2.z);
			
		pMesh->tangent[i]	= Tangent;
		pMesh->tangent[i+1] = Tangent;
		pMesh->tangent[i+2] = Tangent;
	}

	
	
	//Normalize tangents and bitangent
	for (int i = 0; i < pMesh->numVertexes; i++)
	{
		const glm::vec3 & n = pMesh->normals[i];
		const glm::vec3 & t = pMesh->tangent[i];
		pMesh->tangent[i] = glm::orthonormalize(t, n);

		pMesh->binormal[i] = (glm::cross(pMesh->tangent[i], n));
	}
	return res;
}

glm::vec3 CalculateNormals(const std::vector< unsigned int >& vertexIndices, const std::vector<glm::vec3>& temp_vertices, const int currentIndex)
{
	glm::vec3 Edge1 = temp_vertices[vertexIndices[(currentIndex + 1)]] - temp_vertices[vertexIndices[currentIndex]];
	glm::vec3 Edge2 = temp_vertices[vertexIndices[(currentIndex + 2)]] - temp_vertices[vertexIndices[currentIndex]];

	return glm::normalize(glm::cross(Edge1, Edge2));
}


void CalculateTangentAndBinormal(objMesh_t& pMesh, const int currentIndex)
{
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
}


glm::vec3 CalculateSmoothNormals(const std::vector< unsigned int >& vertexIndices, const std::vector<glm::vec3>& temp_vertices, const int currentIndex)
{
	//calculate smooth normals
	glm::vec3 normal(0, 0, 0);
	for (int j = currentIndex; j < vertexIndices.size(); j += 3)
	{
		if (vertexIndices[j] == vertexIndices[currentIndex] && (j + 2) < vertexIndices.size())
		{
			glm::vec3 Edge1 = temp_vertices[vertexIndices[j + 1]] - temp_vertices[vertexIndices[j]];
			glm::vec3 Edge2 = temp_vertices[vertexIndices[j + 2]] - temp_vertices[vertexIndices[j]];

			normal += glm::normalize(glm::cross(Edge1, Edge2));
		}
	}
	return glm::normalize(normal);
}

/*
==============================
OBJ_ParseMesh
Not used
==============================
*/
int OBJ_ParseMesh(FILE* file, const char* objectName) {

	int res = 0;
	while (1) {
		char lineHeader[128];
		res = fscanf(file, "%s", lineHeader);
		if (res == EOF) { break; }

		if (strcmp(lineHeader, "o") == 0) {
			char objectName[60];
			fscanf(file, "%s\n", objectName);
			//res = OBJ_ParseObject(file, objectName, temp_vertices, temp_uvs, temp_normals);
		}
	}
	return res;
}

/*
============================
OBJ_ParseNewMTL - newmtl
============================
*/
void OBJ_ParseNewMTL(FILE* file, objModel_t* model, std::string mtlName) {
	objMaterial_t* material = TYW_NEW objMaterial_t;

	material->map_bump[0] = 0;
	material->map_d[0] = 0;
	material->map_Ka[0] = 0;
	material->map_Kd[0] = 0;
	material->map_Ks[0] = 0;
	material->map_Ns[0] = 0;
	material->count = 0;
	strcpy(material->name, mtlName.c_str());
	while (1) {
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);

		if (res == EOF) { break; }
		else if (strcmp(lineHeader, "newmtl") == 0) {
			char mtlName[60];
			fscanf(file, "%s\n", &mtlName);
			OBJ_ParseNewMTL(file, model, mtlName);
		}

		if (strcmp(lineHeader, "map_Kd") == 0) {
			fscanf(file, "%s\n", &material->map_Kd);
			material->count += 1;
		}
		else if (strcmp(lineHeader, "map_Ks") == 0) {
			fscanf(file, "%s\n", &material->map_Ks);
			material->count += 1;
		}
		else if (strcmp(lineHeader, "map_Bump") == 0) {
			fscanf(file, "%s\n", &material->map_bump);
			material->count += 1;
		}
	}
	model->materials.push_back(material);
}
/*
===================
OBJ_ParseMTL   - material.txt file
map_Ka lenna.tga           # the ambient texture map
map_Kd lenna.tga           # the diffuse texture map (most of the time, it will
#be the same as the ambient texture map)
map_Ks lenna.tga           # specular color texture map
map_Ns lenna_spec.tga      # specular highlight component
map_d lenna_alpha.tga      # the alpha texture map
map_bump lenna_bump.tga    # some implementations use 'map_bump' instead of 'bump' below
bump lenna_bump.tga        # bump map (which by default uses luminance channel of the image)
disp lenna_disp.tga        # displacement map
decal lenna_stencil.tga    # stencil decal texture (defaults to 'matte' channel of the image)
===================
*/
objMaterial_t* OBJ_ParseMTL(const char* fileName) {
	if (objGlobal.model == nullptr) {
		objGlobal.model = TYW_NEW objModel_t;
	}
	objModel_t* model = objGlobal.model;

	FILE* file = fopen(fileName, "r");
	if (file == nullptr) {
		printf("ERROR: Could not load OBJ material file %s", fileName);
		return nullptr;
	}
	while (1) {
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF) { break; }

		if (strcmp(lineHeader, "newmtl") == 0) {
			char mtlName[60];
			fscanf(file, "%s\n", &mtlName);
			OBJ_ParseNewMTL(file, model, mtlName);
		}
	}

	//DEBUG
	for (int i = 0; i < model->materials.size(); i++) {
		objMaterial_t* mat = model->materials[i];
		printf("Material name: %s \r\n", mat->name);
		printf("map_Kd:	   %s  \r\n", mat->map_Kd);
		printf("map_Ks:    %s  \r\n", mat->map_Ks);
		printf("map_Bump:  %s  \r\n", mat->map_bump);
		printf("\r\n");
	}
	return nullptr;
}

/*
===================
OBJ_Parse
===================
*/
objModel_t* OBJ_Parse(FILE* file, std::string& filePath) {
	std::string materialPath = "materials/";
	memset(&objGlobal, 0, sizeof(objGlobal));

	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	objGlobal.currentObject = nullptr;
	objGlobal.model = nullptr;
	objGlobal.model = TYW_NEW objModel_t;
	while (1) {
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF || res == 0) { break; }

		if (strcmp(lineHeader, "mtllib") == 0) {
			char fileName[60];
			fscanf(file, "%s\n", fileName);
			OBJ_ParseMTL((filePath + materialPath + fileName).c_str());
		}
		else if (strcmp(lineHeader, "o") == 0 || strcmp(lineHeader, "g") == 0) {
			char objectName[60];
			fscanf(file, "%s\n", objectName);
			res = OBJ_ParseObject(file, objectName, temp_vertices, temp_uvs, temp_normals);
		}
	}
	return objGlobal.model;
}


/*
===================
OBJ_Load
===================
*/
objModel_t* OBJ_Load(std::string fileName, std::string filePath) {
	objModel_t* obj = nullptr;
	FILE* file = fopen((filePath + fileName).c_str(), "r");
	if (!file) {
		return nullptr;
	}
	obj = OBJ_Parse(file, filePath);
	fclose(file);
	return obj;
}


/*
===================
OBJ_Free
===================
*/
void OBJ_Free(objModel_t* wav) {
	int					i;
	objObject_t			*obj;
	objMesh_t			*mesh;
	objMaterial_t		*material;
	if (!wav) {
		return;
	}
	for (int i = 0; i < wav->objects.size(); i++) {
		obj = wav->objects[i];

		mesh = &obj->mesh;
		SAFE_DELETE_ARRAY(mesh->vertices);
		SAFE_DELETE_ARRAY(mesh->uvs);
		SAFE_DELETE_ARRAY(mesh->normals);
		SAFE_DELETE_ARRAY(mesh->faces);
		SAFE_DELETE(obj);
	}
	wav->objects.clear();

	for (int i = 0; i < wav->materials.size(); i++) {
		material = wav->materials[i];
		SAFE_DELETE(material);
	}
	wav->materials.clear();
	SAFE_DELETE(wav);
}