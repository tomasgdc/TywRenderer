#pragma once
#include "SceneNode.h"

//forward declaration
struct Actor;

/*
AsteroidNode is the root node for asteroid.
It will keep a list of all asteroid actors and will keep information
that is needed to render asteroid.
*/
class AsteroidNode final : public SceneNode
{
public:
	AsteroidNode(void *pTexture);
	~AsteroidNode();

	void VUpdate(long timestamp) override;
	void VRender() override;

	/*
	Not checking if actor is duplicate
	*/
	void AddActorToTheList(const Actor& pActor);

	//Clear Actors
	void DeletActors() { m_itActors.clear(); }

	//Returns const ref characters
	std::vector<Actor>& GetActorsRef() { return m_itActors; }

	//Add new asteroid
	void AddNewAsteroid(Actor&& pActor);

	//Reserves memory for asteroids
	void ReserveMemory(uint32_t size) { m_itActors.reserve(size); }
private:
	std::vector<Actor> m_itActors;
	void *  m_pTexture;
};