#pragma once
#include "Actor.h"
#include <cmath>

inline bool BoundingBoxCollision(const Actor& player, const Actor& asteroid)
{
	if (player.m_x < asteroid.m_x + asteroid.m_size &&
		player.m_x + player.m_size > asteroid.m_size &&
		player.m_y < asteroid.m_y + asteroid.m_size &&
		player.m_y + player.m_size > asteroid.m_size)
	{
		return true;
	}
	return false;
}

inline bool SphereCollision(const Actor& player, const Actor& asteroid)
{
	int dx = player.m_x - asteroid.m_x;
	int dy = player.m_y - asteroid.m_y;
	int distance = sqrt(dx*dx + dy * dy);

	if (distance < player.m_radius + asteroid.m_radius)
	{
		return true;
	}
	return false;
}