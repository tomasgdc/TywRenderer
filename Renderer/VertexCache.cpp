//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#include <RendererPch\stdafx.h>


//Renderer Includes
#include "VertexCache.h"


VertexCache vertexCache;

/*
==============
ClearGeoBufferSet
==============
*/
static void ClearGeoBufferSet(geoBufferSet_t &gbs) {
	gbs.allocations = 0;
}

/*
==============
AllocGeoBufferSet
==============
*/
static void AllocGeoBufferSet(geoBufferSet_t &gbs, const size_t vertexBytes,const size_t indexByes) {
	//gbs.vertexBuffer.AllocateBufferObject(nullptr, vertexBytes, 0);
    //gbs.indexBuffer.AllocateBufferObject(nullptr, indexByes, 1);
}



void VertexCache::Init() 
{
	currentFrame = 0;
	drawListNum = 0;
	listNum = 0;

	for (int i = 0; i < VERTCACHE_NUM_FRAMES; i++) 
	{
		AllocGeoBufferSet(frameData[i], VERTCACHE_VERTEX_MEMORY_PER_FRAME, VERTCACHE_INDEX_MEMORY_PER_FRAME);
	}
	AllocGeoBufferSet(staticData, STATIC_VERTEX_MEMORY, STATIC_INDEX_MEMORY);
}

/*
==============
ActuallyAlloc
==============
*/
void VertexCache::ActuallyAlloc(geoBufferSet_t & vcs, const void * data, size_t bytes, cacheType_t type) {
	//data transfer to GPU
	if (data != nullptr) {
		if (type == cacheType_t::CACHE_VERTEX) {
		//	vcs.vertexBuffer.AllocateBufferObject(data, bytes, 0);
		}
		else if (type == cacheType_t::CACHE_INDEX) {
			//vcs.vertexBuffer.AllocateBufferObject(data, bytes, 1);
		}
		vcs.allocations++;
	}
	else {
		if (type == cacheType_t::CACHE_VERTEX) {
			//vcs.vertexBuffer.AllocateBufferObject(data, bytes, 0);
		}
		else if (type == cacheType_t::CACHE_INDEX) {
			//vcs.vertexBuffer.AllocateBufferObject(data, bytes, 1);
		}
		//vcs.vertexBuffer.AllocateBufferObject(data, bytes, 0);
	}
}

/*
====================
Shutdown
====================
*/
void VertexCache::Shutdown() {
	//This is not finished
	//staticData.vertexBuffer.FreeBufferObject();
}