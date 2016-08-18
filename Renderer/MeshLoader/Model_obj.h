/* ============================================================================
* Tywyl Engine
* Platform:      Windows
* WWW:
* ----------------------------------------------------------------------------
* Copyright 2015 Tomas Mikalauskas. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY TOMAS MIKALAUSKAS ''AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL TOMAS MIKALAUSKAS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The views and conclusions contained in the software and documentation are
* those of the authors and should not be interpreted as representing official
* policies, either expressed or implied, of Tomas Mikalauskas.

DISCLAIMER
The engine design is based on Doom3 BFG engine
https://github.com/id-Software/DOOM-3-BFG.
A lot of tweaks maded to suit my needs.
Tywyll game engine design and functionality will be changed with time.
============================================================================
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
