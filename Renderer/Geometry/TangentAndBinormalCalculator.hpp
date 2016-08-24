/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once


/*
	Calculates Tangent and Binormal
	@param const glm::vec3& v1 - vector 1, const glm::vec3& v2 - vector 2, const glm::vec3& v3 - vector 3, const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, glm::vec3 & n - normal, glm::vec3 & t - tangent, glm::vec3 & b -binormal

	calculate edges
	glm::vec3 Edge1 = v2 - v1;
	glm::vec3 Edge2 = v3 - v1;

	calculate uv edges
	glm::vec2 Edge1Uv = uv2 - uv1;
	glm::vec2 Edge2Uv = uv3 - uv1;
*/
void TangentAndBinormalCalculator(const glm::vec3& Edge1, const glm::vec3& Edge2, const glm::vec2& Edge1Uv, const glm::vec2& Edge2Uv,  glm::vec3 & n, glm::vec3 & t, glm::vec3 & b)
{
	//calculate edges
	//glm::vec3 Edge1 = v2 - v1;
	//glm::vec3 Edge2 = v3 - v1;

	//calculate uv edges
	//glm::vec2 Edge1Uv = uv2 - uv1;
	//glm::vec2 Edge2Uv = uv3 - uv1;


	float r = 1.0f / (Edge1Uv.x * Edge2Uv.y - Edge1Uv.y * Edge2Uv.x);
	glm::vec3 tangent = (Edge1 * Edge2Uv.y - Edge2 * Edge1Uv.y)*r;
	glm::vec3 bitangent = (Edge2 * Edge1Uv.x - Edge1 * Edge2Uv.x)*r;

	t = tangent;
	b = bitangent;

	// Gram-Schmidt orthogonalize
	t = glm::normalize(t - n * glm::dot(n, t));

	// Calculate handedness
	if (glm::dot(glm::cross(n, t), b) < 0.0f) {
		t = t * -1.0f;
	}
}