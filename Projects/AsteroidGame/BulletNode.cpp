#include "BulletNode.h"

//Actor
#include "Actor.h"

//Engine
#include "Engine.h"

//Event
#include "EventManager.h"

//Physics
#include "Physics.h"

//Sond
//#include "SoundManager.h"

#define MAX_BULLETS				60
#define RAPID_FIRE_DELAY_FRAMES 7

BulletNode::BulletNode(void *pTexture) :
	m_pTexture(pTexture),
	m_nextFiredBulletIndex(0),
	m_rapidFireTimer(0)
{
	//Register event
	Engine::getInstance().GetEventManager()->AddListener(std::bind(&BulletNode::OnFireEvent, this), Event::Get_EvtData_OnFire());
}


BulletNode::~BulletNode()
{
	//Delete event
	Engine::getInstance().GetEventManager()->RemoveListener(std::bind(&BulletNode::OnFireEvent, this), Event::Get_EvtData_OnFire());
}






void BulletNode::VUpdate(long dt)
{
	float width = Engine::getInstance().GetWindowsWidth();
	float height = Engine::getInstance().GetWindowsWidth();;

	for (auto& actor : m_itActors)
	{
		if (actor.m_alive)
		{
			actor.m_x += actor.m_vx * dt;
			actor.m_y += actor.m_vy * dt;

			//Check if within screen bonuds
			if (width < actor.m_x + actor.m_size &&
				height < actor.m_y + actor.m_size)
			{
				actor.m_alive = false;
			}
		}
	}
}


void BulletNode::VRender()
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



void BulletNode::OnFireEvent()
{
	/*
	// player firing
	if (m_rapidFireTimer)
	{
	m_rapidFireTimer--;
	}
	if (!IsKeyDown(VK_SPACE))
	{
	m_rapidFireTimer = 0;
	}
	*/

//	if (IsKeyDown(VK_SPACE))
//	{
		m_itActors[m_nextFiredBulletIndex].m_x = m_pOrigin->m_x;
		m_itActors[m_nextFiredBulletIndex].m_y = m_pOrigin->m_y;
		m_itActors[m_nextFiredBulletIndex].m_rz = m_pOrigin->m_rz;

		float const bulletVelocity = 8.0f;
		m_itActors[m_nextFiredBulletIndex].m_vx = -sin(m_itActors[m_nextFiredBulletIndex].m_rz * DEG2RAD) * bulletVelocity + m_pOrigin->m_vx;
		m_itActors[m_nextFiredBulletIndex].m_vy = -cos(m_itActors[m_nextFiredBulletIndex].m_rz * DEG2RAD) * bulletVelocity + m_pOrigin->m_vy;
		m_itActors[m_nextFiredBulletIndex].m_radius = 8;
		m_itActors[m_nextFiredBulletIndex].m_alive = true;

		//m_nextFiredBulletIndex = (m_nextFiredBulletIndex + 1) % MAX_BULLETS;
		//m_rapidFireTimer = RAPID_FIRE_DELAY_FRAMES;
//	}
}

bool BulletNode::CheckCollision(const Actor& pActor)
{
	//TODO:
	//Implement Kd-Tree collision detection using neareset neighbour algorithm
	for (auto& actor : m_itActors)
	{
		if (actor.m_alive)
		{
			if (SphereCollision(pActor, actor))
			{
				actor.m_alive = false;
				return true;
			}

		}
	}

	return false;
}