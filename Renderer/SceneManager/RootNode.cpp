#include <RendererPch\stdafx.h>


//SceneManager Includes
#include "RootNode.h"
#include "RenderPass.hpp"

RootNode::RootNode()
	: SceneNode(0, RenderPass::RenderPass_0,  nullptr)//&Mat4x4::g_Identity)
{
	m_Children.reserve(static_cast<size_t>(RenderPass::RenderPass_Last));


	std::shared_ptr<SceneNode> staticGroup(TYW_NEW SceneNode(0,  RenderPass::RenderPass_Static, nullptr));
	m_Children.push_back(staticGroup);	// RenderPass_Static = 0

	std::shared_ptr<SceneNode> actorGroup(TYW_NEW SceneNode(0,  RenderPass::RenderPass_Actor, nullptr));
	m_Children.push_back(actorGroup);	// RenderPass_Actor = 1

	std::shared_ptr<SceneNode> skyGroup(TYW_NEW SceneNode(0,  RenderPass::RenderPass_Sky, nullptr));
	m_Children.push_back(skyGroup);	// RenderPass_Sky = 2

	std::shared_ptr<SceneNode> invisibleGroup(TYW_NEW SceneNode(0,  RenderPass::RenderPass_NotRendered, nullptr));
	m_Children.push_back(invisibleGroup);	// RenderPass_NotRendered = 3
}



bool RootNode::VAddChild(std::shared_ptr<ISceneNode> kid)
{
	// The Root node has children that divide the scene graph into render passes.
	// Scene nodes will get added to these children based on the value of the
	// render pass member variable.

	RenderPass pass = kid->VGet()->RenderPass();
	if ((unsigned)pass >= m_Children.size() || !m_Children[static_cast<uint32_t>(pass)])
	{
		//assert(0 && "There is no such render pass");
		return false;
	}

	return m_Children[static_cast<uint32_t>(pass)]->VAddChild(kid);
}


bool RootNode::VRemoveChild(uint32_t id)
{
	bool anythingRemoved = false;
	for (uint32_t i = static_cast<uint32_t>(RenderPass::RenderPass_0); i < static_cast<uint32_t>(RenderPass::RenderPass_Last); ++i)
	{
		if (m_Children[i]->VRemoveChild(id))
		{
			anythingRemoved = true;
		}
	}
	return anythingRemoved;
}


bool RootNode::VRenderChildren(SceneManager *pScene)
{
	// This code creates fine control of the render passes.
	for (uint32_t pass = static_cast<uint32_t>(RenderPass::RenderPass_0); pass < static_cast<uint32_t>(RenderPass::RenderPass_Last); ++pass)
	{
		switch (static_cast<RenderPass>(pass))
		{
		case RenderPass::RenderPass_Static:
			break;
		case RenderPass::RenderPass_Actor:
			m_Children[pass]->VRenderChildren(pScene);
			break;
		case RenderPass::RenderPass_Sky:
			//std::shared_ptr<IRenderState> skyPass = pScene->GetRenderer()->VPrepareSkyBoxPass();
			//m_Children[pass]->VRenderChildren(pScene);
			break;
		default:
			break;
		}
	}
	return true;
}