/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>

//Geometry Includes
#include "JointTransform.h"


//Stores into matrix as row major
void JointMat::SetRotation(const glm::mat4x4 &m) 
{
	mat[0][0] = m[0][0];
	mat[0][1] = m[1][0];
	mat[0][2] = m[2][0];
	mat[1][0] = m[0][1];
	mat[1][1] = m[1][1];
	mat[1][2] = m[2][1];
	mat[2][0] = m[0][2];
	mat[2][1] = m[1][2];
	mat[2][2] = m[2][2];
}


glm::mat4x4 JointMat::GetRotation() {
	glm::mat4x4 m;
	
	m[0][0] = mat[0][0];
	m[1][0] = mat[0][1];
	m[2][0] = mat[0][2];
	m[3][0] = 0.0f;

	m[0][1] = mat[1][0];
	m[1][1] = mat[1][1];
	m[2][1] = mat[1][2];
	m[3][1] = 0.0f;

	m[0][2] = mat[2][0];
	m[1][2] = mat[2][1];
	m[2][2] = mat[2][2];
	m[3][2] = 0.0f;

	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
	return m;
}


void JointQuat::CalculateQuatW(glm::quat & q)
{
	float t = 1.0f - (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
	if (t < 0.0f)
	{
		q.w = 0.0f;
	}
	else
	{
		q.w = -sqrtf(t);
	}
}