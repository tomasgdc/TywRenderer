/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once


/*
	Descriptors enum for drawVer
	If you have vert,norm, and uv. Then use for VkBufferObject when subimiting (Vertex | Normal | Uv) etc....
*/
enum drawVertFlags
{
	Vertex   = 1 << 1,
	Normal   = 1 << 2,
	Tangent  = 1 << 3,
	Binormal = 1 << 4,
	Uv       = 1 << 5,
	None     = 1 << 6
};

inline drawVertFlags operator|(const drawVertFlags& a, const drawVertFlags& b)
{
	return static_cast<drawVertFlags>(static_cast<int>(a) | static_cast<int>(b));
}

/*
	Font data
*/
struct drawFont
{
	glm::vec3 vertex;		//12 bytes
	glm::vec2 tex;			//8 bytes
};

/*
	glm::vec3 vertex;		
	glm::vec3 normal;		
	glm::vec3 tangent;	
	glm::vec3 bitangent;
	glm::vec3 color;
	glm::vec2 tex;		
*/
class  drawVert
{
public:
	drawVert(const glm::vec3& v, const glm::vec3& n, const glm::vec3& tangent_, const glm::vec3& bitangent_, const glm::vec3& color_, const glm::vec2& uv);
	drawVert(const glm::vec3& v, const glm::vec3& n, const glm::vec3& tangent, const glm::vec3& bitangent, const glm::vec2& uv);
	drawVert(const glm::vec3& v, const glm::vec3& n, const glm::vec2& t);
	drawVert();

	glm::vec3 vertex;		//12 bytes
	glm::vec3 normal;		//12 bytes
	glm::vec3 tangent;		//12 bytes
	glm::vec3 bitangent;	//12byes
	glm::vec3 color;		//12 bytes;
	glm::vec2 tex;			//8 bytes

	void Clear();
	void SetVertex(float x, float y, float z);
	void SetNormal(float x, float y, float z);
	void SetTexCoords(float x, float y);
};


inline drawVert::drawVert(const glm::vec3& v, const glm::vec3& n, const glm::vec3& tangent_, const glm::vec3& bitangent_, const glm::vec3& color_, const glm::vec2& uv):
vertex(v),
normal(n),
tangent(tangent_),
bitangent(bitangent_),
color(color_),
tex(uv)
{

}

inline drawVert::drawVert(const glm::vec3& v, const glm::vec3& n, const glm::vec3& tangent_, const glm::vec3& bitangent_, const glm::vec2& uv):
	vertex(v),
	normal(n),
	tangent(tangent_),
	bitangent(bitangent_),
	tex(uv)
{

}


inline drawVert::drawVert(const glm::vec3& v, const glm::vec3& n, const glm::vec2& t):
	vertex(v), normal(n), tex(t)
{

}


inline drawVert::drawVert():
	vertex(0.0f, 0.0f, 0.0f),
	normal(0.0f, 0.0f, 0.0f),
	tangent(0.0f, 0.0f, 0.0f),
	bitangent(0.0f, 0.0f, 0.0f),
	color(0.0f, 0.0f, 0.0f),
	tex(0.0f, 0.0f)
{

}


inline void drawVert::Clear()
{
	this->normal = glm::vec3(0.0f, 0.0f, 0.0f);
	this->normal = glm::vec3(0.0f, 0.0f, 0.0f);
	this->tex = glm::vec2(0.0f, 0.0f);
}


inline void drawVert::SetVertex(float x, float y, float z)
{
	this->vertex = glm::vec3(x, y, z);
}


inline void drawVert::SetNormal(float x, float y, float z)
{
	this->vertex = glm::vec3(x, y, z);
}

inline void drawVert::SetTexCoords(float x, float y)
{
	this->tex = glm::vec2(x, y);
}