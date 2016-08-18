#include <RendererPch\stdafx.h>

//SceneManager Includes
#include "CameraNode.h"
#include "SceneNode.h"
#include "SceneManager.h"
#include "SceneNodeProperties.h"


SceneNode::SceneNode(uint32_t actorId, RenderPass renderPass, const glm::mat4x4 *to, const glm::mat4x4 *from)
{
	m_pParent = NULL;
	m_Props.m_ActorId = actorId;
	m_Props.m_Name = "SceneNode";
	m_Props.m_RenderPass = renderPass;
	//VSetTransform(to, from);
	SetRadius(0);

	// [mrmike] - these lines were moved to VOnRestore() post press
	//Color color = (renderComponent) ? renderComponent->GetColor() : g_White;
	//m_Props.m_Material.SetDiffuse(color);
}


SceneNode::~SceneNode()
{
}

	
bool SceneNode::VOnRestore(SceneManager *pScene)
{
	//glx::vec4<float> color = (m_RenderComponent) ? m_RenderComponent->GetColor() : g_White;
	//m_Props.m_Material.SetDiffuse(color);

	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	//VOnRestore(pScene);

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();
	while (i != end)
	{
		(*i)->VOnRestore(pScene);
		++i;
	}
	return S_OK;
}


//
// SceneNode::VOnLostDevice	
bool SceneNode::VOnLostDevice(SceneManager *pScene)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// VOnRestore()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();
	while (i != end)
	{
		(*i)->VOnLostDevice(pScene);
		++i;
	}
	return S_OK;
}

//
// SceneNode::VSetTransform	
void SceneNode::VSetTransform(const glm::mat4x4 *toWorld, const glm::mat4x4 *fromWorld)
{
	m_Props.m_ToWorld = *toWorld;
	if (!fromWorld)
	{
		//m_Props.m_ToWorld = glm::determinant(m_Props.m_ToWorld);
		//m_Props.m_FromWorld = glm::inverse(m_Props.m_FromWorld);
	}
	else
	{
		m_Props.m_FromWorld = *fromWorld;
	}
}


//
// SceneNode::VPreRender		
bool SceneNode::VPreRender(SceneManager *pScene)
{
	pScene->PushAndSetMatrix(m_Props.m_ToWorld);
	return S_OK;
}

//
// SceneNode::VPostRender
bool SceneNode::VPostRender(SceneManager *pScene)
{
	pScene->PopMatrix();
	return S_OK;
}



//
// SceneNode::VIsVisible		
bool SceneNode::VIsVisible(SceneManager *pScene) const
{
	// transform the location of this node into the camera space 
	// of the camera attached to the scene

	glm::mat4x4 toWorld, fromWorld;
	//pScene->GetCamera()->VGet()->Transform(&toWorld, &fromWorld);

	glm::vec3 pos = GetWorldPosition();


	glm::translate(fromWorld, pos);

	//Frustum const &frustum = pScene->GetCamera()->GetFrustum();

	bool isVisible = false; // frustum.Inside(fromWorldPos, VGet()->Radius());
	return isVisible;
}

//
// SceneNode::GetWorldPosition			- not described in the book
//
//   This was added post press to respect any SceneNode ancestors - you have to add all 
//   their positions together to get the world position of any SceneNode.
//
const glm::vec3 SceneNode::GetWorldPosition() const
{
	glm::vec3 pos = GetPosition();
	if (m_pParent)
	{
		pos += m_pParent->GetWorldPosition();
	}
	return pos;
}


//
// SceneNode::VOnUpdate			
bool SceneNode::VOnUpdate(SceneManager *pScene, DWORD const elapsedMs)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// VOnUpdate()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		(*i)->VOnUpdate(pScene, elapsedMs);
		++i;
	}
	return S_OK;
}



glm::vec3 SceneNode::GetDirection() const
{

	/*
	[RT.x] [UP.x] [BK.x] [POS.x]
	[RT.y] [UP.y] [BK.y] [POS.y]
	[RT.z] [UP.z] [BK.z] [POS.Z]
	[    ] [    ] [    ] [US   ]
	*/

	glm::mat4 dir = m_Props.ToWorld();
	return glm::vec3(dir[0][3] + dir[2][0], dir[1][3] + dir[2][1], dir[2][3] + dir[2][2]);
}



bool SceneNode::VRemoveChild(uint32_t id)
{
	for (SceneNodeList::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		const SceneNodeProperties* pProps = (*i)->VGet();
		if (pProps->ActorId() != 0 && id == pProps->ActorId())
		{
			i = m_Children.erase(i);	//this can be expensive for vectors
			return true;
		}
	}
	return false;
}



bool SceneNode::VPick(SceneManager *pScene, RayCast *raycast)
{
	for (SceneNodeList::const_iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		HRESULT hr = (*i)->VPick(pScene, raycast);

		if (hr == E_FAIL)
			return E_FAIL;
	}

	return S_OK;
}


void SceneNode::SetAlpha(float alpha)
{
	m_Props.SetAlpha(alpha);
	for (SceneNodeList::const_iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		std::shared_ptr<SceneNode> sceneNode = std::static_pointer_cast<SceneNode>(*i);
		sceneNode->SetAlpha(alpha);
	}
}

glm::vec3 SceneNode::GetPosition() const
{
	glm::mat4 pos = m_Props.ToWorld();
	return glm::vec3(pos[0][3], pos[1][3], pos[2][3]);
}


//
// SceneNode::VRenderChildren			
bool SceneNode::VRenderChildren(SceneManager *pScene)
{
	// Iterate through the children....
	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		if ((*i)->VPreRender(pScene) == S_OK)
		{
			// You could short-circuit rendering
			// if an object returns E_FAIL from
			//VPreRender(pScene);

			// Don't render this node if you can't see it
			//if ((*i)->VIsVisible(pScene))
			//{
			float alpha = 0.0f;
			(*i)->VRender(pScene);

			if (alpha == 1.0f)
			{

			}
			/*
			else if (alpha != 1.0f)
			{
			// The object isn't totally transparent...
			AlphaSceneNode *asn = TYW_NEW AlphaSceneNode;
			assert(asn);
			asn->m_pNode = *i;
			asn->m_Concat = pScene->GetTopMatrix();

			glx::vec4<float> worldPos(asn->m_Concat.GetPosition());

			glx::mat4<float> fromWorld = pScene->GetCamera()->VGet()->FromWorld();

			glx::vec4<float> screenPos;// = fromWorld.Xform(worldPos);

			asn->m_ScreenZ = screenPos.z;

			pScene->AddAlphaSceneNode(asn);
			}
			*/

			// [mrmike] see comment just below...
			(*i)->VRenderChildren(pScene);
		}

		// [mrmike] post-press fix - if the parent is not visible, the childrend
		//           shouldn't be visible either.
		//(*i)->VRenderChildren(pScene);
		//}
		(*i)->VPostRender(pScene);
		++i;
	}

	return S_OK;
}


//
// SceneNode::VAddChild
bool SceneNode::VAddChild(std::shared_ptr<ISceneNode> ikid)
{
	m_Children.push_back(ikid);

	std::shared_ptr<SceneNode> kid = std::static_pointer_cast<SceneNode>(ikid);

	kid->m_pParent = this;					// [mrmike] Post-press fix - the parent was never set!

											// The radius of the sphere should be fixed right here
	glm::mat4x4 pos = kid->VGet()->ToWorld();
	glm::vec3 kidPos(pos[0][3], pos[1][3], pos[2][3]);

	// [mrmike] - Post-press fix. This was not correct! subtracting the parents's position from the kidPos
	//            created a HUGE radius, depending on the location of the parent, which could be anywhere
	//            in the game world.

	//Vec3 dir = kidPos - m_Props.ToWorld().GetPosition();
	//float newRadius = dir.Length() + kid->VGet()->Radius();

	float newRadius = 0;// = kidPos.Length() + kid->VGet()->Radius();

	if (newRadius > m_Props.m_Radius)
		m_Props.m_Radius = newRadius;

	return true;
}