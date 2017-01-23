#include "Engine.h"

//Renderer Includes
#include "Renderer.h"

//Event Manager
#include "EventManager.h"


//SceneManager
#include "SceneManager.h"


//Other includes
#include <string>

Engine::Engine() :m_bInitialized(false)
{
	m_strApplicationName = "Set name for application";
}

Engine::~Engine()
{
	Shutdown();
}

void Engine::Initialize(int WindowWidth, int WindowsHeight, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM), bool bFullscreen)
{
	m_pRenderer = new Renderer;
	//m_pAudioComponent = new AudioComponent;
	m_pEventManager = new EventManager;
	m_pSceneManager = new SceneManager;
	//m_pSoundManager = new SoundManager;

	m_iScreenWidth = WindowWidth;
	m_iScreenHeight = WindowsHeight;


	//Initialize QueryPerfomance counter
	QueryPerformanceCounter(&m_StartTime);
	QueryPerformanceFrequency(&m_Freq);

	// Initialize Renderer	
	if (m_pRenderer->VInitRenderer(WindowWidth, WindowsHeight, false, HandleWindowMessages));
	{

	}

	//Everything is initialized
	m_bInitialized = true;
}

void Engine::Frame()
{


}

void Engine::Shutdown()
{
	if (m_bInitialized)
	{
		//The order of deletion is important

		delete m_pSceneManager;
		m_pSceneManager = nullptr;

		delete m_pAudioComponent;
		m_pAudioComponent = nullptr;

		delete m_pRenderer;
		m_pRenderer = nullptr;

		delete m_pEventManager;
		m_pEventManager = nullptr;

		delete m_pSoundManager;
		m_pSoundManager = nullptr;

		//UnregisterClass(m_strApplicationName.c_str(), m_wc.hInstance);
	}
}

int	Engine::GetTimeInMS()
{
	LARGE_INTEGER ms;
	QueryPerformanceCounter(&ms);
	return ((ms.QuadPart - m_StartTime.QuadPart) * 1000) / m_Freq.QuadPart;
}

void Engine::GetMousePos(float &x, float &y)
{
	//POINT p;
	//GetCursorPos(&p);
	//ScreenToClient(m_hWnd, &p);
	//x = p.x;
	//y = p.y;
}