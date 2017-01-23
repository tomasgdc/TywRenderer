#include "SceneManager.h"
#include "SceneNode.h"

//Actor
#include "Actor.h"


SceneManager::SceneManager()
{
	m_pRootNode = new SceneNode;
}


SceneManager::~SceneManager()
{
	if (m_pRootNode != nullptr)
	{
		delete m_pRootNode;
		m_pRootNode = nullptr;
	}
}


ISceneNode* SceneManager::AddChild(uint32_t ActorId, ISceneNode * pActor)
{
	auto& pIter = m_ActorMap.find(ActorId);
	if (pIter == m_ActorMap.end())
	{
		m_ActorMap.insert(std::make_pair(ActorId, pActor));
		m_pRootNode->VAddChildren(pActor);
		return pActor;
	}
	else
	{
		//hmmmmmmm
		delete pActor;
		pActor = nullptr;
	}
	return nullptr;
}

ISceneNode*  SceneManager::GetChild(uint32_t ActorId)
{
	auto& pIter = m_ActorMap.find(ActorId);
	if (pIter != m_ActorMap.end())
	{
		return pIter->second;
	}

	return nullptr;
}


void SceneManager::Render()
{
	m_pRootNode->VRender();
}

void SceneManager::Update(long timestamp)
{
	m_pRootNode->VUpdate(timestamp);
}
