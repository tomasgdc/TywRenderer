#include <RendererPch\stdafx.h>


//SceneManager Includes
#include "CameraNode.h"
#include "SceneManager.h"
#include "RootNode.h"





SceneManager::SceneManager(std::shared_ptr<IRenderer> renderer)
{
	m_Root.reset(TYW_NEW RootNode());
	m_Renderer = renderer;

	//D3DXCreateMatrixStack(0, &m_MatrixStack);

	// [mrmike] - event delegates were added post-press
	//	IEventManager* pEventMgr = IEventManager::Get();
	//	pEventMgr->VAddListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EvtData_New_Render_Component::sk_EventType);
	//	pEventMgr->VAddListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EvtData_Destroy_Actor::sk_EventType);
	//	pEventMgr->VAddListener(MakeDelegate(this, &Scene::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);
	//	pEventMgr->VAddListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EvtData_Modified_Render_Component::sk_EventType);
}

//
// Scene::~Scene					- Chapter 16, page 539
//
SceneManager::~SceneManager()
{
	// [mrmike] - event delegates were added post-press!
	//EventManager* pEventMgr = EngineSub
	//	pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EvtData_New_Render_Component::sk_EventType);
	//	pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EvtData_Destroy_Actor::sk_EventType);
	//	pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);

	//	pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EvtData_Modified_Render_Component::sk_EventType);

	//	SAFE_RELEASE(m_MatrixStack);
}

//
// Scene::OnRender					- Chapter 16, page 539
//
HRESULT SceneManager::OnRender()
{
	// The render passes usually go like this 
	// 1. Static objects & terrain
	// 2. Actors (dynamic objects that can move)
	// 3. The Sky
	// 4. Anything with Alpha

	if (m_Root && m_Camera)
	{
		// The scene root could be anything, but it
		// is usually a SceneNode with the identity
		// matrix

		m_Camera->SetViewTransform(this);

		//	m_LightManager->CalcLighting(this);
		if (m_Root->VPreRender(this) == S_OK)
		{
			m_Root->VRender(this);
			m_Root->VRenderChildren(this);
			m_Root->VPostRender(this);
		}
		RenderAlphaPass();
	}
	return S_OK;
}

//
// Scene::OnLostDevice						- not in the book
//
//    All Scene nodes implement VOnLostDevice, which is called in the D3D9 renderer.
//
HRESULT SceneManager::OnLostDevice()
{
	if (m_Root)
	{
		return m_Root->VOnLostDevice(this);
	}
	return S_OK;
}

//
// Scene::OnRestore					- Chapter 16, page 540
//
HRESULT SceneManager::OnRestore()
{
	if (!m_Root)
		return S_OK;

	return m_Root->VOnRestore(this);
}





bool SceneManager::AddChild(uint32_t id, std::shared_ptr<ISceneNode> kid)
{
	if (id != 0)
	{
		// This allows us to search for this later based on actor id
		m_ActorMap[id] = kid;
	}

	//std::shared_ptr<LightNode> pLight = std::dynamic_pointer_cast<LightNode>(kid);
	//if (pLight != nullptr && m_LightManager->m_Lights.size() + 1 < MAXIMUM_LIGHTS_SUPPORTED)
	//{
	//	m_LightManager->m_Lights.push_back(pLight);
	//}


	return m_Root->VAddChild(kid);
}

bool SceneManager::RemoveChild(uint32_t id)
{
	if (id == 0)
		return false;
	std::shared_ptr<ISceneNode> kid = FindActor(id);
	//std::shared_ptr<LightNode> pLight = std::dynamic_pointer_cast<LightNode>(kid);
	//if (pLight != NULL)
	//{
		//m_LightManager->m_Lights.remove(pLight);
	//}
	m_ActorMap.erase(id);
	return m_Root->VRemoveChild(id);
}


//
// Scene::OnUpdate					- Chapter 16, page 540
//
HRESULT SceneManager::OnUpdate(const int deltaMilliseconds)
{
	if (!m_Root)
		return S_OK;

	//	static DWORD lastTime = timeGetTime();
	//	DWORD elapsedTime = 0;
	//	DWORD now = timeGetTime();

	//	elapsedTime = now - lastTime;
	//	lastTime = now;

	return m_Root->VOnUpdate(this, 0);
}

//
// Scene::FindActor					- Chapter 16, page 542
//
std::shared_ptr<ISceneNode> SceneManager::FindActor(uint32_t id)
{
	SceneActorMap::iterator i = m_ActorMap.find(id);
	if (i == m_ActorMap.end())
	{
		return std::shared_ptr<ISceneNode>();
	}

	return i->second;
}


//
// Scene::RenderAlphaPass			- Chapter 16, page 543
//
void SceneManager::RenderAlphaPass()
{
	/*
	std::shared_ptr<IRenderState> alphaPass = m_Renderer->VPrepareAlphaPass();

	m_AlphaSceneNodes.sort();
	while (!m_AlphaSceneNodes.empty())
	{
	AlphaSceneNodes::reverse_iterator i = m_AlphaSceneNodes.rbegin();
	PushAndSetMatrix((*i)->m_Concat);
	(*i)->m_pNode->VRender(this);
	delete (*i);
	PopMatrix();
	m_AlphaSceneNodes.pop_back();
	}
	*/
}