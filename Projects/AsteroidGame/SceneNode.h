#pragma once
#include "ISceneNode.h"
#include <vector>

//////////////////////////////////////////////////////////////
//
// SceneNodeList
//
//   Every scene node has a list of its children - this 
//   is implemented as a vector since it is expected that
//   we won't add/delete nodes very often, and we'll want 
//   fast random access to each child.
//
//////////////////////////////////////////////////////////////
typedef std::vector<ISceneNode*> SceneNodeList;

/*
Each SceneNode manages it's own memory by calling DeleteNode
*/
class SceneNode : public ISceneNode
{
protected:
	SceneNodeList				 m_Children;

public:
	SceneNode();
	virtual ~SceneNode();

	virtual void VUpdate(long miliseconds);
	virtual void VRender();
	virtual void VAddChildren(ISceneNode* pSceneNode);
};