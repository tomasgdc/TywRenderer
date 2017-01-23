#include "ShipNode.h"

//Actor
#include "Actor.h"

//Engine
#include "Engine.h"


//Event Manager
#include "EventManager.h"

//Physics
#include "Physics.h"

ShipNode::ShipNode(void *pTexture) :m_pTexture(pTexture)
{
	m_pActor = new Actor;

	//Register event
	Engine::getInstance().GetEventManager()->AddListener(std::bind(&ShipNode::HandleInput, this), Event::Get_EvtData_ActorMove());
}


ShipNode::~ShipNode()
{
	//delete actor
	delete m_pActor;
	m_pActor = nullptr;

	//Deregister event
	Engine::getInstance().GetEventManager()->RemoveListener(std::bind(&ShipNode::HandleInput, this), Event::Get_EvtData_ActorMove());
}


void ShipNode::VUpdate(long dt)
{
	if (m_pActor->m_alive)
	{
		m_pActor->m_x += m_pActor->m_vx * dt;
		m_pActor->m_y += m_pActor->m_vy * dt;
		m_pActor->m_x = m_pActor->m_x < 0 ? Engine::getInstance().GetWindowsWidth() : m_pActor->m_x > Engine::getInstance().GetWindowsWidth() ? 0 : m_pActor->m_x;
		m_pActor->m_y = m_pActor->m_y < 0 ? Engine::getInstance().GetWindowsHeight() : m_pActor->m_y > Engine::getInstance().GetWindowsHeight() ? 0 : m_pActor->m_y;
	}
}


void ShipNode::VRender()
{
	if (m_pActor->m_alive)
	{
		//Engine::getInstance().GetRenderer()->VRenderGFX(m_pTexture,
		//	m_pActor->m_x, m_pActor->m_y, m_pActor->m_radius, m_pActor->m_radius, m_pActor->m_rz * DEG2RAD);
	}
}

void ShipNode::HandleInput()
{
	// update player orientation & position
//	m_pActor->m_rz += IsKeyDown(VK_RIGHT) ? -5 : IsKeyDown(VK_LEFT) ? 5 : 0;
	float const acceleration = 0.125f;
//	m_pActor->m_vx += (sin(m_pActor->m_rz * DEG2RAD)) * -(IsKeyDown(VK_UP) * acceleration);
//	m_pActor->m_vy += (cos(m_pActor->m_rz * DEG2RAD)) * -(IsKeyDown(VK_UP) * acceleration);

	float const maxVelocity = 5.0f;
	float velocitySqrd = m_pActor->m_vx*m_pActor->m_vx + m_pActor->m_vy*m_pActor->m_vy;
	if (velocitySqrd > maxVelocity*maxVelocity)
	{
		float len = 1.0f / sqrtf(velocitySqrd);
		m_pActor->m_vx = m_pActor->m_vx * len * maxVelocity;
		m_pActor->m_vy = m_pActor->m_vy * len * maxVelocity;
	}
}

void ShipNode::ResetPlayer(const Actor& actor)
{
	m_pActor->m_x = actor.m_x;
	m_pActor->m_y = actor.m_y;
	m_pActor->m_vx = actor.m_vx;
	m_pActor->m_vy = actor.m_vy;
	m_pActor->m_rz = actor.m_rz;
	m_pActor->m_radius = actor.m_radius;
	m_pActor->m_alive = actor.m_alive;
}


bool ShipNode::CheckCollision(const Actor& pActor)
{
	if (m_pActor->m_alive)
	{
		if (SphereCollision(pActor, *m_pActor))
		{
			m_pActor->m_alive = false;
			return true;
		}
	}
	return false;
}