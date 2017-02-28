/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
#include <External\vulkan\vulkan.h>
#include <Renderer\Geometry\VertData.h>
#include <Renderer\Geometry\JointTransform.h>



//forwad declared
class Material;


// our only drawing geometry type
struct  srfTriangles_t 
{
	int					numVerts;		//number of vertices
	drawVert*			verts;			//vert, normal, tex

	int					numIndexes;		//number of indixes
	uint32_t*			indexes;		//vertex indices
};

struct  modelSurface_t 
{
	int					id;
	uint32_t			numMaterials;
	Material*			material;
	srfTriangles_t*		geometry;
};

struct  MD5Joint {
	char								name[256];
	int									parentID;
};

// deformable meshes precalculate as much as possible from a base frame, then generate
// complete srfTriangles_t from just a new set of vertexes
struct  deformInfo_t 
{
	int					numSourceVerts;

	// numOutputVerts may be smaller if the input had duplicated or degenerate triangles
	// it will often be larger if the input had mirrored texture seams that needed
	// to be busted for proper tangent spaces
	int					numOutputVerts;
	drawVert *			verts;

	int					numIndexes;
	uint32_t *			indexes;

	uint32_t*			silIndexes;				// indexes changed to be the first vertex with same XYZ, ignoring normal and texcoords

	int					numMirroredVerts;		// this many verts at the end of the vert list are tangent mirrors
	int *				mirroredVerts;			// tri->mirroredVerts[0] is the mirror of tri->numVerts - tri->numMirroredVerts + 0

	int					numDupVerts;			// number of duplicate vertexes
	int *				dupVerts;				// pairs of the number of the first vertex and the number of the duplicate vertex
};

class  RenderModel 
{
public:
	virtual ~RenderModel() {};

	// used for initial loads, reloadModel, and reloading the data of purged models
	// Upon exit, the model will absolutely be valid, but possibly as a default model
	virtual void				LoadModel() = 0;

	//Loads only static models
	virtual void				InitFromFile(std::string fileName, std::string filePath) = 0;


	//Allocate triangle data
	virtual srfTriangles_t *	AllocSurfaceTriangles(int numVerts, int numIndexes) const = 0;

	// Frees surfaces triangles.
	virtual void				FreeSurfaceTriangles(srfTriangles_t *tris) = 0;

	// returns a static model
	virtual RenderModel *		InstantiateDynamicModel() = 0;

	// returns the name of the model
	virtual const char	*		getName() const = 0;

	// reports the amount of memory (roughly) consumed by the model
	virtual int					getSize() const = 0;	

	//Deletes any data that was stored in class. 
	//Each derived class can have different clearing implementation
	virtual void				Clear(VkDevice device) = 0;
};