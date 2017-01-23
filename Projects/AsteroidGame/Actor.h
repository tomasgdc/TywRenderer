#pragma once

struct Actor
{
	float	m_x, m_y;		// position
	float	m_rz;			// orientation (in degrees)
	float	m_vx, m_vy;		// linear velocity
	float	m_avz;			// angular velocity
	float	m_radius;
	float	m_mass;
	int		m_size;			// aka radius index
	bool	m_alive;		// dead or alive
};
