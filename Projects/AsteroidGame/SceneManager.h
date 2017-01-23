#pragma once
#include <map>
#include <vector>


//forward declaration
class ISceneNode;
class SceneNode;

//First is actorId
typedef std::map<uint32_t, ISceneNode*> SceneActorMap;

/*
Scene manager provides adding, rendering and update via root node.
Also provides fast lookup for specific actor by it's id
*/
class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	ISceneNode*  AddChild(uint32_t ActorId, ISceneNode* pActor);
	ISceneNode*  GetChild(uint32_t ActorId);

	void Render();
	void Update(long timestamp);

private:
	SceneActorMap m_ActorMap;
	SceneNode*	  m_pRootNode;
};