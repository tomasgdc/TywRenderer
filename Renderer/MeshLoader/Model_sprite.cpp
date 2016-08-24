/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>


//Renderer Includes
#include "Model_local.h"


/*
========================
InstantiateDynamicModel
========================
*/
RenderModel *	RenderModelSprite::InstantiateDynamicModel()
{
	RenderModelStatic* staticModel;
	srfTriangles_t *tri;

	//alloc 
	staticModel = TYW_NEW RenderModelStatic;

	//alloc space
	tri = staticModel->AllocSurfaceTriangles(6, 0);

	//1 triangle
	tri->verts[0].Clear();
	tri->verts[0].SetVertex(0.0f, 0.0f, 0.0f);
	tri->verts[0].SetTexCoords(0.0f, 0.0f);

	tri->verts[1].Clear();
	tri->verts[1].SetVertex(0.0f, 1.0f, 0.0f);
	tri->verts[1].SetTexCoords(0.0f, 1.0f);

	tri->verts[2].Clear();
	tri->verts[2].SetVertex(1.0f, 0.0f, 0.0f);
	tri->verts[2].SetTexCoords(1.0f, 0.0f);

	//2 triangle
	tri->verts[3].Clear();
	tri->verts[3].SetVertex(1.0f, 0.0f, 0.0f);
	tri->verts[3].SetTexCoords(1.0f, 0.0f);

	tri->verts[4].Clear();
	tri->verts[4].SetVertex(0.0f, 1.0f, 0.0f);
	tri->verts[4].SetTexCoords(0.0f, 1.0f);

	tri->verts[5].Clear();
	tri->verts[5].SetVertex(1.0f, 1.0f, 0.0f);
	tri->verts[5].SetTexCoords(1.0f, 1.0f);

	//memcpy
	staticModel->Memcpy(tri, sizeof(srfTriangles_t));
	return staticModel;



}