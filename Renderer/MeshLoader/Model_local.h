/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
#include "Model.h"



/*
================================================================================

RenderModelStatic

================================================================================
*/
class TYWRENDERER_API RenderModelStatic: public RenderModel
{
public:
								RenderModelStatic();
	virtual						~RenderModelStatic() {}

	virtual void				InitFromFile(std::string fileName, std::string filePath);
	virtual void				LoadModel();
	virtual srfTriangles_t *	AllocSurfaceTriangles(int numVerts, int numIndexes) const;
	virtual void				FreeSurfaceTriangles(srfTriangles_t *tris);
	virtual RenderModel *		InstantiateDynamicModel();
	void						Memcpy(srfTriangles_t * tri, size_t size);

	bool						OBJLoad(std::string& fileName, std::string& filePath);
	bool						ConvertOBJToModelSurfaces(const struct objModel_a* obj);

	virtual void				Clear(VkDevice device);
	virtual const char	*		getName() const;
	virtual int					getSize() const;
	void						setName(std::string name);
	void						setMaterial(std::string name, Material* mat);
	void						addSurface(modelSurface_t& surface);
public:
	std::unordered_map<std::string, Material*>				m_material;
	std::unordered_map<std::string, Material*>::iterator	it;

	std::string										name;
	std::vector<modelSurface_t>						surfaces;

	//Stores number of materials loaded. Hack around till will figure out how to go around this problem.
	std::unordered_map<std::string, uint32_t> materialSizes;
};

/*
===============================================================================

MD5 animated model

===============================================================================
*/




struct triangleIndex_t
{
	uint32_t					indices[3];
};

struct vertexWeight_t
{
	uint32_t					jointId;
	glm::vec3					pos;
	float						jointWeight;
};


struct vertIndex_t
{
	uint32_t					index;
	uint32_t					firstWeightForVertex;
	uint32_t					numWeightsForVertex;
	glm::vec2					texCoord;
};


class TYWRENDERER_API MD5Mesh 
{
	friend class				RenderModelMD5;
public:
	MD5Mesh();
	~MD5Mesh();

	void						ParseMesh(FILE* file, int numJoints, std::vector<JointQuat>& joints);//std::vector<JointMat>& joints);

	int							NumVerts() const { return numVerts; }
	int							NumTris() const { return numTris; }

	void						UpdateMesh(const MD5Mesh *mesh, std::vector<JointQuat>& joints, const	JointMat *entJointsInverted, deformInfo_t *surf);
	//void						CalculateBounds(const idJointMat * entJoints, idBounds & bounds) const;
	//int						NearestJoint(int a, int b, int c) const;

public:
	struct meshStructure
	{
		//At the moment we only have v/uv
		glm::vec4   boneWeight1;
		glm::vec4   boneWeight2;
		glm::ivec4	boneId1;
		glm::ivec4	boneId2;
		glm::vec3   vertex;
		glm::vec3	normal;
		glm::vec3	tangent;
		glm::vec3	binormal;
		glm::vec2   tex;
	};


	std::vector<vertIndex_t>	 verts;
	std::vector<vertexWeight_t>  weight;
	std::vector<meshStructure>	 deformInfosVec;		//replacement for deforminto_t*
	std::vector<uint32_t>		 indexes;

	uint32_t					numVerts;			// number of vertices
	uint32_t					numTris;			// number of triangles
	uint32_t					numMeshJoints;		// number of mesh joints
	uint32_t					surfaceNum;			// number of the static surface created for this mesh
	uint32_t					numMaterials;		// number of materials
	float						maxJointVertDist;	// maximum distance a vertex is separated from a joint
	deformInfo_t *				deformInfo;			// used to create srfTriangles_t from base frames and new vertexes
	int8_t *					meshJoints;			// the joints used by this mesh
	Material *					shader;				// material applied to mesh
};

class TYWRENDERER_API RenderModelMD5 final: public RenderModelStatic 
{
public:
	void				InitFromFile(std::string fileName, std::string filePath);
	void				Clear(VkDevice device) override;
	//virtual bool				LoadBinaryModel(idFile * file, const ID_TIME_T sourceTimeStamp);
	//virtual void				WriteBinaryModel(idFile * file, ID_TIME_T *_timeStamp = NULL) const;
	//virtual dynamicModel_t	IsDynamicModel() const;
	//virtual idBounds			Bounds(const struct renderEntity_s *ent) const;
	//virtual void				Print() const;
	//virtual void				List() const;
	//virtual void				TouchData();
	//virtual void				PurgeModel();
	//virtual void				LoadModel();
	//virtual int					Memory() const;
	//virtual RenderModel *		InstantiateDynamicModel(const struct renderEntity_s *ent, const viewDef_t *view, idRenderModel *cachedModel);
	//virtual int					NumJoints() const;
	//virtual const MD5Joint *	GetJoints() const;
	//virtual jointHandle_t		GetJointHandle(const char *name) const;
	//virtual const char *		GetJointName(jointHandle_t handle) const;
	//virtual const idJointQuat *	GetDefaultPose() const;
	//virtual int					NearestJoint(int surfaceNum, int a, int b, int c) const;

	//virtual bool				SupportsBinaryModel() { return true; }

public:
	std::vector<MD5Joint>	 joints;
	std::vector<JointQuat>	 defaultPose;
	std::vector<MD5Mesh>	 meshes;
	std::vector<glm::mat4x4> inverseBindPose;
	
	

	//void						DrawJoints(const renderEntity_t *ent, const viewDef_t *view) const;
	bool						ParseJoint(FILE* ptrfile, MD5Joint& joint, JointQuat& pose);
};

/*
================================================================================

RenderModelSprite

================================================================================
*/
class TYWRENDERER_API RenderModelSprite: public RenderModel 
{
public:
	virtual RenderModel *		InstantiateDynamicModel();
};



/*
================================================================================

RenderModelSprite

================================================================================
*/
class TYWRENDERER_API RenderModelAssimp : public RenderModel
{
public:
	RenderModelAssimp();
	~RenderModelAssimp();

	void				InitFromFile(std::string fileName, std::string filePath) override;
	
	void				LoadModel() override;
	srfTriangles_t *	AllocSurfaceTriangles(int numVerts, int numIndexes) const override;
	void				FreeSurfaceTriangles(srfTriangles_t *tris) override;
	RenderModel *		InstantiateDynamicModel() override;
	const char	*		getName() const override;
	int				    getSize() const override;
	void				Clear(VkDevice device) override;

public:
	typedef std::unordered_map<std::string, Material*> MaterialMap;
	MaterialMap					m_material;
	MaterialMap::iterator		it;


	std::vector<modelSurface_t> m_Entries;
	struct Dimension
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);
		glm::vec3 size;
	} dim;
};


