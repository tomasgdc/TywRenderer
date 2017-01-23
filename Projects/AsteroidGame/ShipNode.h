#pragma once
#include "SceneNode.h"

//forward declaration
struct Actor;

/*
ShipNode
*/
class ShipNode final : public SceneNode
{
public:
	ShipNode(void *pTexture);
	~ShipNode();

	void VUpdate(long miliseconds) override;
	void VRender() override;

	void HandleInput();
	void ResetPlayer(const Actor& actor);
	Actor* GetActor() { return m_pActor; }

	//Check collision against current character
	bool CheckCollision(const Actor& pActor);
private:
	Actor*	m_pActor;
	void *  m_pTexture;
};