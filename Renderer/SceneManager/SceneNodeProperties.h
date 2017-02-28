#pragma once


//forward declaration
enum class RenderPass: uint32_t;

/*
	class SceneNodeProperties
	This class is contained in the SceneNode class, and gains
	easy access to common scene node properties such as its ActorId
	or render pass, with a single ISceneNode::VGet() method.
*/
class  SceneNodeProperties
{
	friend class SceneNode;

protected:
	uint32_t                m_ActorId;
	std::string				m_Name;
	glm::mat4x4		        m_ToWorld, m_FromWorld;
	float					m_Radius;
	RenderPass				m_RenderPass;
	//Material				m_Material;
	//AlphaType				m_AlphaType;

	void SetAlpha(const float alpha)
	{
		//m_AlphaType = AlphaMaterial; m_Material.SetAlpha(alpha);
	}

public:
	SceneNodeProperties(void);
	const uint32_t &ActorId() const { return m_ActorId; }
	glm::mat4x4 const &ToWorld() const { return m_ToWorld; }
	glm::mat4x4 const &FromWorld() const { return m_FromWorld; }
	void Transform(glm::mat4x4 *toWorld, glm::mat4x4 *fromWorld) const;

	const char * Name() const { return m_Name.c_str(); }

	//bool HasAlpha() const { return m_Material.HasAlpha(); }
	//float Alpha() const { return m_Material.GetAlpha(); }
	//AlphaType AlphaType() const { return m_AlphaType; }

	RenderPass RenderPass() const { return m_RenderPass; }
	float Radius() const { return m_Radius; }
	//Material GetMaterial() const { return m_Material; }
};
