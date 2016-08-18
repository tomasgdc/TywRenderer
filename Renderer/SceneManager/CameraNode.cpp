#include <RendererPch\stdafx.h>


//SceneManager Includes
#include "CameraNode.h"
#include "SceneManager.h"


bool CameraNode::VRender(SceneManager *pScene)
{
	if (m_DebugCamera)
	{
		pScene->PopMatrix();

		//m_Frustum.Render();

		pScene->PushAndSetMatrix(m_Props.ToWorld());
	}

	return S_OK;
}

//
// CameraNode::VOnRestore				- Chapter 16, page 550
//
bool CameraNode::VOnRestore(SceneManager *pScene)
{
	//m_Frustum.SetAspect(DXUTGetWindowWidth() / (FLOAT)DXUTGetWindowHeight());
	//D3DXMatrixPerspectiveFovLH(&m_Projection, m_Frustum.m_Fov, m_Frustum.m_Aspect, m_Frustum.m_Near, m_Frustum.m_Far);

	//g_pRendGL->VSetProjectionTransform(&m_Projection);
	return S_OK;
}


HRESULT CameraNode::SetViewTransform(SceneManager *pScene)
{
	//If there is a target, make sure the camera is
	//rigidly attached right behind the target
	if (m_pTarget)
	{
		glm::mat4x4 mat = m_pTarget->VGet()->ToWorld();
		glm::vec4 at = m_CamOffsetVector;
		glm::vec4 atWorld;// = mat.Xform(at);
		glm::vec3 pos;// = mat.GetPosition() + Vec3(atWorld);
		glm::translate(mat, pos);
		VSetTransform(&mat);
	}
	m_View = VGet()->FromWorld();
	m_View = glm::lookAt(glm::vec3(0, 0, 8), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	return S_OK;
}


// Returns the concatenation of the world and view projection, which is generally sent into vertex shaders
glm::mat4x4 CameraNode::GetWorldViewProjection(SceneManager *pScene)
{
	glm::mat4x4 world = pScene->GetTopMatrix();
	glm::mat4x4 view = VGet()->FromWorld();
	glm::mat4x4 worldView = world * view;
	return worldView * m_Projection;
}
