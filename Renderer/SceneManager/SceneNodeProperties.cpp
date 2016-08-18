#include <RendererPch\stdafx.h>


//SceneManager Includes
#include "SceneNode.h"
#include "SceneNodeProperties.h"


SceneNodeProperties::SceneNodeProperties(void)
{
	m_ActorId = static_cast<uint32_t>(RenderPass::RenderPass_0);
	m_Radius = 0;
	m_RenderPass = RenderPass::RenderPass_0;
}


//
// SceneNodeProperties::Transform	
void SceneNodeProperties::Transform(glm::mat4x4 *toWorld, glm::mat4x4 *fromWorld) const
{
	if (toWorld)
		*toWorld = m_ToWorld;

	if (fromWorld)
		*fromWorld = m_FromWorld;
}