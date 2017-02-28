/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#ifndef _VERTEX_CACHE_H_
#define _VERTEX_CACHE_H_

//forward declared
class IndexBuffer;
class VertexBuffer;


const int VERTCACHE_INDEX_MEMORY_PER_FRAME = 31 * 1024 * 1024;
const int VERTCACHE_VERTEX_MEMORY_PER_FRAME = 31 * 1024 * 1024;

const int STATIC_INDEX_MEMORY = 31 * 1024 * 1024;
const int STATIC_VERTEX_MEMORY = 31 * 1024 * 1024;

const int VERTCACHE_NUM_FRAMES = 2;


enum class cacheType_t {
	CACHE_VERTEX,
	CACHE_INDEX
};


struct  geoBufferSet_t
{

//	IndexBuffer	 indexBuffer;
//	VertexBuffer vertexBuffer;
	int32_t		 indexMemUsed;
	int32_t		 vertexMemUsed;
	int			 allocations; //number of vertexBuffers
};


class  VertexCache
{
public:
	void			Init();
	void			Shutdown();

	//data is going to be available till next map load
	void			AllocStaticVertex(const void * data, int bytes);

public:
	int				currentFrame;	// for determining the active buffers
	int				listNum;		// currentFrame % VERTCACHE_NUM_FRAMES
	int				drawListNum;	// (currentFrame-1) % VERTCACHE_NUM_FRAMES
	
	geoBufferSet_t	staticData;
	geoBufferSet_t	frameData[VERTCACHE_NUM_FRAMES];

	void			ActuallyAlloc(geoBufferSet_t & vcs, const void * data, size_t bytes, cacheType_t type);
};

extern	 VertexCache	vertexCache;
#endif