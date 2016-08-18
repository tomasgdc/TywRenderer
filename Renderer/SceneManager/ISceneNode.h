#pragma once

//forward declaration
class SceneNodeProperties;
class SceneManager;
class RayCast;

class ISceneNode
{
public:
	virtual const SceneNodeProperties * const VGet() const = 0;

	virtual void VSetTransform(const  glm::mat4x4 *toWorld, const glm::mat4x4 *fromWorld = nullptr) = 0;

	virtual bool VOnUpdate(SceneManager *pScene, DWORD const elapsedMs) = 0;
	virtual bool VOnRestore(SceneManager *pScene) = 0;

	virtual bool VPreRender(SceneManager *pScene) = 0;
	virtual bool VIsVisible(SceneManager *pScene) const = 0;
	virtual bool VRender(SceneManager *pScene) = 0;
	virtual bool VRenderChildren(SceneManager *pScene) = 0;
	virtual bool VPostRender(SceneManager *pScene) = 0;

	virtual bool VAddChild(std::shared_ptr<ISceneNode> kid) = 0;
	virtual bool VRemoveChild(uint32_t id) = 0;
	virtual bool VOnLostDevice(SceneManager *pScene) = 0;
	virtual bool VPick(SceneManager *pScene, RayCast *pRayCast) = 0;


	virtual ~ISceneNode() { };
};