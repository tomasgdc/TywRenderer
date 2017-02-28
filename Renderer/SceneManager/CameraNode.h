#pragma once
#include "SceneNode.h"


//forward declaration
class SceneManager;


////////////////////////////////////////////////////
//
// class CameraNode
//
//    A camera node controls the D3D view transform and holds the view
//    frustum definition
//
////////////////////////////////////////////////////
class  CameraNode: public SceneNode
{
public:
	CameraNode(glm::mat4x4 const *t)
		: SceneNode(0, RenderPass::RenderPass_0, t),
		m_bActive(true),
		m_DebugCamera(false),
		m_pTarget(std::shared_ptr<SceneNode>()),
		m_CamOffsetVector(0.0f, 1.0f, -10.0f, 0.0f)
	{
	}

	virtual bool VRender(SceneManager *pScene);
	virtual bool VOnRestore(SceneManager *pScene);
	virtual bool VIsVisible(SceneManager *pScene) const { return m_bActive; }

	//const Frustum &GetFrustum() { return m_Frustum; }
	
	void SetTarget(std::shared_ptr<SceneNode> pTarget)
	{
		m_pTarget = pTarget;
	}
	void ClearTarget() { m_pTarget = std::shared_ptr<SceneNode>(); }
	std::shared_ptr<SceneNode> GetTarget() { return m_pTarget; }

	glm::mat4x4 GetWorldViewProjection(SceneManager *pScene);
	HRESULT SetViewTransform(SceneManager *pScene);

	glm::mat4x4 GetProjection() { return m_Projection; }
	glm::mat4x4 GetView() { return m_View; }

	void SetCameraOffset(const glm::vec4 & cameraOffset)
	{
		m_CamOffsetVector = cameraOffset;
	}

protected:
	//Frustum			m_Frustum;
	glm::mat4x4		m_Projection;
	glm::mat4x4		m_View;
	bool			m_bActive;
	bool			m_DebugCamera;
	std::shared_ptr<SceneNode> m_pTarget;
	glm::vec4		m_CamOffsetVector;	//Direction of camera relative to target.
};
