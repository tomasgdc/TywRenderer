#pragma once
#include "SceneNode.h"

//forward declaration
struct Actor;
class IEventData;

/*
BulletNode is the root node for asteroid.
It will keep a list of all asteroid actors and will keep information
that is needed to render bullet.

Bullet node could be made as particle node . With lifespan and other fancy particle things.

Something like this maybe ?
ISceneNode->SceneNode->ParticleSystem->BulletNode (... hmmm... nope. nope ... nope)
Too many VTable's and everything would get messier.

Maybe this? (Use composion and aggregation)
ISceneNode->SceneNode->BulletNode
{
struct Bullet
{
ParticleDetails ?
}
}
*/
class BulletNode final : public SceneNode
{
public:
	BulletNode(void *pTexture);
	~BulletNode();

	void VUpdate(long timestamp) override;
	void VRender() override;

	//Reserve bullets size
	void ReserveBullets(int size) { m_itActors.resize(size); }

	//Clear Actors
	void DeletActors() { m_itActors.clear(); }

	//Needed for bullet location (from where bullets will be fired )
	void AddBulletLocationOrigin(Actor* pActor) { m_pOrigin = pActor; }


	//During fire event
	void OnFireEvent();

	//Check collision against current character
	bool CheckCollision(const Actor& pActor);
private:
	std::vector<Actor> m_itActors;
	void *  m_pTexture;
	Actor*	m_pOrigin;

	// bullet firing state
	int m_nextFiredBulletIndex;
	int m_rapidFireTimer;
};