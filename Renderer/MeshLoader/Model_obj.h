/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#ifndef _MODEL_OBJ_
#define _MODEL_OBJ_

/*
================================================================================

Wavefront object (.obj)

================================================================================
*/

typedef struct {
	unsigned int				vertexNum[3];
	unsigned int				tVertexNum[3];
}objFace_t;

typedef struct{
	//Verts
	glm::vec3*				vertices;
	uint32_t				numVertexes;

	//Uvs
	glm::vec2*				uvs;
	uint32_t				numUvs;

	//Normals
	glm::vec3*				normals;
	uint32_t				numNormals;

	//Tangents
	glm::vec3*				tangent;
	uint32_t				numTangets;

	//Binormals
	glm::vec3*				binormal;
	uint32_t				numBinormals;

	//Faces
	int				numFaces;
	objFace_t*		faces;
}objMesh_t;


typedef struct {
	char    name[128];		     //material name
	char	map_Ka[128];		 //ambient
	char    map_Kd[128];		 //diffuse, could be same as ambient
	char	map_Ks[128];		 //specular color
	char	map_Ns[128];		 //specular highlight			
	char    map_d[128];			 //alpha
	char    map_bump[128];		 //normal
	int		count;				 //number of textures
}objMaterial_t;

typedef struct {
	char		    name[128];		//object name
	char			mat_name[128];	//material name
	objMesh_t		mesh;
}objObject_t;


typedef struct objModel_a{
	std::vector<objMaterial_t*>	materials;
	std::vector<objObject_t* >	objects;
}objModel_t;

objModel_t*	OBJ_Load(std::string fileName, std::string filePath);
void		OBJ_Free(objModel_t* obj);
#endif
