#pragma once
#include "SceneNode.h"



//    This is the root node of the scene graph.
//
////////////////////////////////////////////////////
class TYWRENDERER_API RootNode: public SceneNode
{
public:
	RootNode();
	virtual bool VAddChild(std::shared_ptr<ISceneNode> kid);
	virtual bool VRenderChildren(SceneManager *pScene);
	virtual bool VRemoveChild(uint32_t id);
	virtual bool VIsVisible(SceneManager *pScene) const { return true; }
};
