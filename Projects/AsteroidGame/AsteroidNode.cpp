#include "AsteroidNode.h"

//Actor
#include "Actor.h"

//Engine
#include "Engine.h"


#define NUM_INTIAL_ASTEROIDS	10
#define MAX_ASTEROIDS	 		(NUM_INTIAL_ASTEROIDS*4)


AsteroidNode::AsteroidNode(void *pTexture) :m_pTexture(pTexture)
{

}


AsteroidNode::~AsteroidNode()
{
	//Delete Resources
	//TODO: Resources could be handled somewhere else
	if (m_pTexture != nullptr)
	{
		//delete m_pTexture;
		//m_pTexture = nullptr;
	}
}


void AsteroidNode::VUpdate(long dt)
{
	for (auto& actor : m_itActors)
	{
		if (actor.m_alive)
		{
			actor.m_x += actor.m_vx * dt;
			actor.m_y += actor.m_vy * dt;
			actor.m_x = actor.m_x < 0 ? Engine::getInstance().GetWindowsWidth() : actor.m_x > Engine::getInstance().GetWindowsWidth() ? 0 : actor.m_x;
			actor.m_y = actor.m_y < 0 ? Engine::getInstance().GetWindowsHeight() : actor.m_y > Engine::getInstance().GetWindowsHeight() ? 0 : actor.m_y;
		}
	}
}


void AsteroidNode::VRender()
{
	for (auto& actor : m_itActors)
	{
		if (actor.m_alive)
		{
			//Engine::getInstance().GetRenderer()->VRenderGFX(m_pTexture,
			//	actor.m_x, actor.m_y, actor.m_radius, actor.m_radius, actor.m_rz * DEG2RAD);
		}
	}
}

void AsteroidNode::AddActorToTheList(const Actor& pActor)
{
	m_itActors.push_back(pActor);
}

void AsteroidNode::AddNewAsteroid(Actor&& pActor)
{
	//Stay within the limits
	if (m_itActors.size() + 1 > MAX_ASTEROIDS)return;

	m_itActors.push_back(std::move(pActor));
}