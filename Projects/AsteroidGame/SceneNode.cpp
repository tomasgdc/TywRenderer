#include "SceneNode.h"


SceneNode::SceneNode()
{

}


SceneNode::~SceneNode()
{
	for (auto child : m_Children)
	{
		if (child != nullptr)
		{
			delete child;
			child = nullptr;
		}
	}
}


void SceneNode::VUpdate(long miliseconds)
{
	for (auto child : m_Children)
	{
		child->VUpdate(miliseconds);
	}
}

void SceneNode::VRender()
{
	/*
	TODO: Add new method ISVisbiel which checks if current object is inside frustrum (current camera).
	*/

	for (auto child : m_Children)
	{
		child->VRender();
	}
}


void SceneNode::VAddChildren(ISceneNode* pSceneNode)
{
	m_Children.push_back(pSceneNode);
}

