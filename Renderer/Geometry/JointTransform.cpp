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

glm::vec3 JointMat::GetTranslation()
{
	glm::vec3 t;
	t[0] = mat[0][3];
	t[1] = mat[1][3];
	t[2] = mat[2][3];
	return t;
}
