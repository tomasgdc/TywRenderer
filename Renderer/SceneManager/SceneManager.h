#pragma once


//forward declaration
class SceneNode;
class CameraNode;
class IRenderer;


/*
SceneManager manages SceneNodes. It creates new nodes, detaches existing ones,
traverses scenegraph and renders it.
*/
class TYWRENDERER_API SceneManager
{
protected:
	std::shared_ptr<SceneNode>  	m_Root;
	std::shared_ptr<CameraNode> 	m_Camera;
	std::shared_ptr<IRenderer>		m_Renderer;

	//AlphaSceneNodes 				m_AlphaSceneNodes;
	SceneActorMap 					m_ActorMap;
	//LightManager					*m_LightManager;
	void RenderAlphaPass();

public:
	SceneManager(std::shared_ptr<IRenderer> renderer = nullptr);
	virtual ~SceneManager();

	HRESULT OnRender();
	HRESULT OnRestore();
	HRESULT OnLostDevice();
	HRESULT OnUpdate(const int deltaMilliseconds);
	std::shared_ptr<ISceneNode> FindActor(uint32_t id);
	bool AddChild(uint32_t id, std::shared_ptr<ISceneNode> kid);
	bool RemoveChild(uint32_t id);


	void SetCamera(std::shared_ptr<CameraNode> camera) { m_Camera = camera; }
	const std::shared_ptr<CameraNode> GetCamera() const { return m_Camera; }


	void PushAndSetMatrix(const glm::mat4x4 &toWorld)
	{
		// Note this code carefully!!!!! It is COMPLETELY different
		// from some DirectX 9 documentation out there....
		// Scene::PushAndSetMatrix - Chapter 16, page 541

		//m_MatrixStack->Push();
		//m_MatrixStack->MultMatrixLocal(&toWorld);
		glm::mat4x4 mat = GetTopMatrix();
		//m_Renderer->VSetWorldTransform(&mat);
	}

	void PopMatrix()
	{
		// Scene::PopMatrix - Chapter 16, page 541
		//m_MatrixStack->Pop();
		glm::mat4x4 mat = GetTopMatrix();
		//m_Renderer->VSetWorldTransform(&mat);
	}

	const glm::mat4x4 GetTopMatrix()
	{
		// Scene::GetTopMatrix - Chapter 16, page 541
		//return static_cast<const glx::mat4<float>>(*m_MatrixStack->GetTop());

		const glm::mat4x4 t;
		return t;
	}

	///LightManager *GetLightManager() { return m_LightManager; }
	//void AddAlphaSceneNode(AlphaSceneNode *asn) { m_AlphaSceneNodes.push_back(asn); }

	HRESULT Pick(RayCast *pRayCast) { return m_Root->VPick(this, pRayCast); }
	std::shared_ptr<IRenderer> GetRenderer() { return m_Renderer; }
};