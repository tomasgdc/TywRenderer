#pragma once
#include "ISceneNode.h"
#include "RenderPass.hpp"
#include "SceneNodeProperties.h"


//forward declaration
class SceneManager;
class SceneNodeProperties;





/*
	SceneNodeList
		Every scene node has a list of its children - this 
		is implemented as a vector since it is expected that
		we won't add/delete nodes very often, and we'll want 
		fast random access to each child.
*/
typedef std::vector<std::shared_ptr<ISceneNode> > SceneNodeList;


/*
 SceneActorMap Description
 An STL map that allows fast lookup of a scene node given an ActorId.
 */
typedef std::map<uint32_t, std::shared_ptr<ISceneNode> > SceneActorMap;


//////////////////////////////////////////////////////////////
//
// SceneNode
//
//   Implements ISceneNode. Forms the base class for any node
//   that can exist in the 3D scene graph managed by class Scene.
//
//////////////////////////////////////////////////////////////

class  SceneNode : public ISceneNode
{
	friend class Scene;

protected:
	SceneNodeList				 m_Children;
	SceneNode					*m_pParent;
	SceneNodeProperties			 m_Props;
	//WeakBaseRenderComponentPtr	 m_RenderComponent;

public:
	SceneNode(uint32_t actorId, RenderPass renderPass, const glm::mat4x4 *to, const glm::mat4x4 *from = nullptr);

	virtual ~SceneNode();

	virtual const SceneNodeProperties* const VGet() const { return &m_Props; }

	virtual void VSetTransform(const glm::mat4x4 *toWorld, const glm::mat4x4 *fromWorld = NULL);

	virtual bool VOnRestore(SceneManager *pScene);
	virtual bool VOnUpdate(SceneManager *, DWORD const elapsedMs);

	virtual bool VPreRender(SceneManager *pScene);
	virtual bool VIsVisible(SceneManager *pScene) const;
	virtual bool VRender(SceneManager *pScene) { return S_OK; }
	virtual bool VRenderChildren(SceneManager *pScene);
	virtual bool VPostRender(SceneManager *pScene);

	virtual bool VAddChild(std::shared_ptr<ISceneNode> kid);
	virtual bool VRemoveChild(uint32_t id);
	virtual bool VOnLostDevice(SceneManager *pScene);
	virtual bool VPick(SceneManager *pScene, RayCast *pRayCast);

	void SetAlpha(float alpha);
	float GetAlpha() const { return 0; }

	glm::vec3 GetPosition() const;
	void SetPosition(const glm::vec3 &pos) {  }

	const glm::vec3 GetWorldPosition() const;					// [mrmike] added post-press to respect ancestor's position 

	glm::vec3 GetDirection() const;

	void SetRadius(const float radius) { m_Props.m_Radius = radius; }
//	void SetMaterial(const Material &mat) { m_Props.m_Material = mat; }
};

