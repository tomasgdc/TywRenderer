#pragma once
class JointQuat 
{
public:
	glm::quat		 q;
	glm::vec3		 t;
};

class JointMat 
{
public:
	void					SetRotation(const glm::mat4x4	&m);
	glm::mat4x4				GetRotation();
	void					SetTranslation(const glm::vec3	&t);
	glm::vec3				GetTranslation();

	
	glm::vec3 operator * (glm::vec3& v) const;
public:
	float mat[3][4];
};


//multiplication operator
inline glm::vec3 JointMat::operator * (glm::vec3& v) const
{
	glm::vec3 vec
		(
		mat[0][0] * v.x + mat[1][0] * v.y + mat[2][0] * v.z,
		mat[0][1] * v.x + mat[1][1] * v.y + mat[2][1] * v.z,
		mat[0][2] * v.x + mat[1][2] * v.y + mat[2][2] * v.z);

	//vec.x = vec.x / mat[0][3];
	//vec.y = vec.y / mat[1][3];
	//vec.z = vec.z / mat[2][3];
	return vec;
}


inline void JointMat::SetTranslation(const glm::vec3 &t) {
	mat[0][3] = t[0];
	mat[1][3] = t[1];
	mat[2][3] = t[2];
}