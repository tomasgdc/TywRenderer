//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#include <RendererPch\stdafx.h>


//Renderer Includes
#include "MeshLoader\Model.h"
#include "VKRenderer.h"


inline srfTriangles_t* R_AllocStaticTriSurf()
{
	srfTriangles_t* tri = TYW_NEW srfTriangles_t;
	return tri;
}

inline void R_AllocStaticTriSurfVerts(srfTriangles_t *tri, int numVerts) 
{
	if (!tri)return;
	tri->numVerts = numVerts;
	tri->verts = TYW_NEW drawVert[numVerts];
}
inline void R_AllocStaticTriSurfIndexes(srfTriangles_t *tri, int numIndexes) 
{
	if (!tri)return;
	tri->numIndexes = numIndexes;
	tri->indexes = TYW_NEW uint32_t[numIndexes]; //use 16bit alloc for indixes
}

inline void	R_FreeStaticTriSurfSilIndexes(srfTriangles_t *tri)
{
	if (!tri)return;
	SAFE_DELETE_ARRAY(tri->indexes);
}

inline void	R_FreeStaticTriSurf(srfTriangles_t *tri)
{
	if (tri == nullptr)return;

	SAFE_DELETE_ARRAY(tri->verts);
	SAFE_DELETE_ARRAY(tri->indexes);
	SAFE_DELETE(tri);
}

inline void	R_FreeStaticTriSurfVerts(srfTriangles_t *tri)
{
	if (!tri)return;
	SAFE_DELETE_ARRAY(tri->verts);
}

inline void R_CreateStaticBuffersForTri(srfTriangles_t & tri) 
{

}